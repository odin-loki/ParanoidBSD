#!/usr/bin/env bash
# Proof gate (AGENTS.md section 10). Replaces the PRISM hook: PRISM is a probabilistic model checker
# with no C/C++ front end, so it cannot check PBSD code. Usage: proof_check.sh <task-id> <worktree>
#
# The tier comes from the task definition (PBSD_PROOF_TIER, set by the runner, never from the tree),
# and selects:
#   T1  Lean 4 (lake build) or TLA+ (Apalache) on proofs/<WP>/model/, then the axiom audit and the
#       statement comparator against the locked statements in proofs/<WP>/spec/ (hash in proofs/spec-lock.json)
#   T2  ESBMC in contract / k-induction mode on every obligation in proofs/<WP>/<U>/obligations.json,
#       plus the vacuity run and the annotation-only diff
#   T3  ESBMC (C++) or CBMC (C) bounded with unwinding assertions; GenMC for files marked concurrent
#   T4  CBMC/ESBMC gateway harnesses under proofs/<WP>/<U>/harness/
# Every tool runs with LOCKED flags from this script or the worker image, never from the agent.
# Vacuity: the verification build defines PBSD_VACUITY; the dialect's PBSD_PRE macros then add
# assert(false) after the preconditions and PBSD_REACHED() becomes assert(false), so a check that
# still "passes" proves nothing and is rejected.
# Exit 0 pass, 1 fail, 3 infrastructure error. Results are printed as "PBSD-RESULT: <json>" lines
# (PROVED / BOUNDED), which the runner keeps with the gate event for the proof report.
# The tool command lines below are the starting point; P0-07 pins the tool versions and the
# calibration run confirms each flag against them (AGENTS.md section 12, step 6).
set -uo pipefail
id="$1"; wt="$2"
wp="${id%%-U*}"; wp="${wp%-MODEL}"; wp="${wp%-INT}"; unit="$(sed -E 's/^.*-(U[0-9]+)-.*$/\1/' <<<"$id")"
if [[ "$id" == *-INT ]]; then
  # Integrator: re-check every unit whose code or proofs the integration changed (tiers from the
  # pinned task list, never from the tree).
  tasks="${PBSD_TASKS:-/opt/pbsd/tasks.json}"; [ -f "$tasks" ] || { echo "infrastructure: $tasks missing"; exit 3; }
  base="$(git -C "$wt" log -1 --format=%B | sed -n 's/^PBSD-Base: //p')"; [ -n "$base" ] || exit 3
  changed="$(git -C "$wt" diff --name-only "$base" HEAD)"
  rc=0
  while IFS=$'\t' read -r pid ptier own; do
    grep -qF "$own" <<<"$changed" || continue
    echo "== re-check $pid ($ptier)"; PBSD_PROOF_TIER="$ptier" "$0" "$pid" "$wt" || rc=1
  done < <(jq -r --arg wp "$wp" '.[] | select(.wp==$wp and .role=="prover" and .proof_tier!=null and (.id|endswith("-MODEL")|not)) | [.id, .proof_tier, .owns[1]] | @tsv' "$tasks")
  exit $rc
fi
tier="${PBSD_PROOF_TIER:-}"
[[ "$id" == *-MODEL ]] && tier=T1
[ -n "$tier" ] || { echo "infrastructure: PBSD_PROOF_TIER not set by the runner"; exit 3; }
LOCK="$wt/proofs/spec-lock.json"
fail() { echo "FAIL: $*"; exit 1; }
need() { command -v "$1" >/dev/null || { echo "infrastructure: $1 not installed"; exit 3; }; }
T="${PBSD_PROOF_TIMEOUT_S:-1800}"

# ---- banned constructs, everywhere the task can write proofs or annotations
banned_lean='(^|[^A-Za-z_])(sorry|admit|native_decide|implemented_by|extern)([^A-Za-z_]|$)|^[[:space:]]*axiom[[:space:]]|debug\.skipKernelTC|set_option[[:space:]]+(debug|trust)'
banned_c='__CPROVER_assume|__ESBMC_assume|__VERIFIER_assume|#pragma[[:space:]]+(CPROVER|ESBMC)'
scan() { grep -rnEI "$2" $1 2>/dev/null | grep -v '/harness/skeleton/' && fail "banned construct (see lines above)"; return 0; }

annotation_only() {   # the code with PBSD_* annotation lines removed must equal the reviewed code
  local base; base="$(git -C "$wt" log -1 --format=%B | sed -n 's/^PBSD-Base: //p')"
  [ -n "$base" ] || { echo "no PBSD-Base trailer: cannot check annotation-only diff"; exit 3; }
  local strip='/^[[:space:]]*(PBSD_(PRE|POST|ASSERT|INVARIANT|DECREASES|ASSIGNS|GHOST)\b|\/\/ *@proof)/d'
  while IFS= read -r f; do
    case "$f" in proofs/*) continue ;; esac
    diff <(git -C "$wt" show "$base:$f" 2>/dev/null | sed -E "$strip") <(sed -E "$strip" "$wt/$f") >/dev/null \
      || fail "$f changes code, not only annotations"
  done < <(git -C "$wt" diff --name-only "$base" HEAD -- . ':!proofs')
}

case "$tier" in
T0)
  echo "T0: static checks only (compile-time asserts run in the build step)"; exit 0 ;;
T1)
  m="$wt/proofs/$wp/model"; s="$wt/proofs/$wp/spec"
  [ -d "$m" ] || fail "no model in proofs/$wp/model"
  [ -f "$LOCK" ] || fail "proofs/spec-lock.json missing (written by P0-FREEZE)"
  # Locked statements unchanged since the freeze.
  while IFS=$'\t' read -r path hash; do
    [ "$(sha256sum "$wt/$path" | cut -d' ' -f1)" = "$hash" ] || fail "locked statement file $path changed"
  done < <(jq -r --arg wp "$wp" 'to_entries[] | select(.key | startswith("proofs/"+$wp+"/spec/")) | [.key,.value] | @tsv' "$LOCK")
  scan "$m" "$banned_lean"
  if ls "$m"/*.tla >/dev/null 2>&1; then
    need apalache-mc
    for f in "$m"/*.tla; do   # inductive invariant: Init => IndInv, IndInv /\ Next => IndInv', IndInv => Safety
      timeout "$T" apalache-mc check --init=Init --inv=IndInv --length=0 "$f" || fail "Init does not imply IndInv in $f"
      timeout "$T" apalache-mc check --init=IndInv --inv=IndInv --length=1 "$f" || fail "IndInv not inductive in $f"
      timeout "$T" apalache-mc check --init=IndInv --inv=Safety --length=0 "$f" || fail "IndInv does not imply Safety in $f"
    done
  fi
  if [ -f "$m/lakefile.lean" ] || [ -f "$m/lakefile.toml" ]; then
    need lake
    (cd "$m" && timeout "$T" lake build) || fail "lake build"
    # Axiom audit and statement comparator: every locked statement Spec.X must be proved as Model.X.
    chk="$m/.pbsd_check.lean"; : > "$chk"
    echo "import Model" >> "$chk"; echo "import Spec" >> "$chk"
    jq -r --arg wp "$wp" '.theorems[$wp][]? // empty' "$LOCK" | while read -r th; do
      echo "example : Spec.$th := Model.$th" >> "$chk"; echo "#print axioms Model.$th" >> "$chk"
    done
    out="$(cd "$m" && timeout "$T" lake env lean "$chk" 2>&1)" || { echo "$out" | tail -40; fail "a proved statement does not match the locked one"; }
    rm -f "$chk"
    bad="$(grep -oE "depends on axioms: \[[^]]*\]" <<<"$out" | tr -d '[]' | tr ',' '\n' | sed 's/.*axioms: //;s/ //g' \
           | grep -vE '^(propext|Classical\.choice|Quot\.sound)?$' || true)"
    [ -z "$bad" ] || fail "proofs use axioms outside propext/Classical.choice/Quot.sound: $bad"
  fi
  echo "PROVED: $wp model"; exit 0 ;;
T2|T3|T4)
  d="$wt/proofs/$wp/$unit"; [ -d "$d" ] || fail "no proofs/$wp/$unit"
  ob="$d/obligations.json"; [ -f "$ob" ] || fail "no obligations.json (the list of functions and properties to check)"
  [ "$(jq 'length' "$ob")" -gt 0 ] || fail "empty obligation list"
  # Assumptions are banned in code and in T2/T3 obligations. T4 harnesses may constrain their inputs
  # with assumptions (that is what a harness is), so they must end with PBSD_REACHED(); and the
  # vacuity run below (which turns PBSD_REACHED into assert(false)) must fail for every harness.
  if [ "$tier" = T4 ]; then scan "$d --exclude-dir=harness" "$banned_c"; else scan "$d" "$banned_c"; fi
  scan "$wt/pbsd" "$banned_c"
  [ "$tier" = T4 ] || annotation_only
  need jq; results="[]"
  while IFS=$'\t' read -r file fn kind; do
    src="$wt/$file"; [ -f "$src" ] || fail "obligation file $file missing"
    case "$kind:$tier" in
      *:T2)  need esbmc; flags=(${PBSD_ESBMC_T2_FLAGS:---std c++20 --function "$fn" --enforce-contract "$fn" --k-induction --max-k-step 64 --unlimited-k-steps --overflow-check --unsigned-overflow-check --memory-leak-check --timeout ${T}s}) ;;
      concurrent:*) need genmc; flags=() ;;
      *:T3|*:T4)
        if [[ "$file" == *.c ]]; then need cbmc; flags=(${PBSD_CBMC_FLAGS:---function "$fn" --bounds-check --pointer-check --div-by-zero-check --signed-overflow-check --unsigned-overflow-check --pointer-overflow-check --conversion-check --undefined-shift-check --unwind 32 --unwinding-assertions})
        else need esbmc; flags=(${PBSD_ESBMC_T3_FLAGS:---std c++20 --function "$fn" --unwind 32 --unwinding-assertions --overflow-check --unsigned-overflow-check --timeout ${T}s}); fi ;;
    esac
    tool=esbmc; [[ "$kind" == concurrent ]] && tool=genmc; [[ "$file" == *.c && "$tier" != T2 ]] && tool=cbmc
    inc=(-I "$wt/pbsd/kernel/include" -I "$wt/pbsd/kernel/base" -DPBSD_VERIFY=1)
    if [ "$tool" = genmc ]; then out="$(timeout "$T" genmc -rc11 "${inc[@]}" "$src" 2>&1)"; rc=$?
    else out="$(timeout "$T" "$tool" "$src" "${inc[@]}" "${flags[@]}" 2>&1)"; rc=$?; fi
    echo "$out" | tail -15
    [ $rc -eq 124 ] && fail "$fn: checker timeout"
    grep -qE 'Generated 0 VCC|0 remaining after simplification|No properties to check' <<<"$out" && fail "$fn: zero verification conditions"
    [ $rc -eq 0 ] || fail "$fn ($tool) did not verify"
    if [ "$tool" != genmc ]; then   # vacuity: with assert(false) after the preconditions (or at PBSD_REACHED) the check must FAIL
      [ "$tier" != T4 ] || grep -q 'PBSD_REACHED' "$src" || fail "$fn: harness does not end with PBSD_REACHED();"
      timeout "$T" "$tool" "$src" "${inc[@]}" -DPBSD_VACUITY=1 "${flags[@]}" >/dev/null 2>&1
      [ $? -ne 0 ] || fail "$fn: assumptions or preconditions are unsatisfiable (vacuity check passed)"
    fi
    status=$([ "$tier" = T2 ] && echo PROVED || echo "BOUNDED")
    results="$(jq -c --arg f "$file" --arg fn "$fn" --arg s "$status" --arg t "$tool" '. + [{file:$f, function:$fn, status:$s, tool:$t}]' <<<"$results")"
  done < <(jq -r '.[] | [.file, .function, (.kind // "sequential")] | @tsv' "$ob")
  echo "PBSD-RESULT: $results"
  echo "$tier: $(jq length <<<"$results") obligation(s) $( [ "$tier" = T2 ] && echo PROVED || echo BOUNDED )"; exit 0 ;;
*) fail "unknown tier $tier" ;;
esac
