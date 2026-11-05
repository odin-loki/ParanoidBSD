#!/usr/bin/env bash
# Proof gate (AGENTS.md section 10): PRISM is the checker. Usage: proof_check.sh <task-id> <worktree>
#
# PRISM (odin-loki/PRISM, binary `prism`) runs its stages on the unit and writes report.json; this
# script decides pass or fail from the findings and adds the checks PRISM cannot make about an agent
# (locked statements, annotation-only diffs, banned constructs, tier from the task).
#   T1  proofs/<WP>/model/: Lean 4 (lake build + axiom audit + comparator against the locked
#       statements) or TLA+ (Apalache). The Lean proofs themselves come from `prism prove`
#       (FORGE + the Goedel prover model), called by the agent through the lean_prove tool.
#   T2  PRISM stages contracts, wp, pir, esbmc, harness, review on the unit: every obligation must be
#       PROVED / PROVED-UNBOUNDED / PROVED-CERTIFIED / PROVED-ASSUMING, none VACUOUS-ASSUMPTION or FAILED
#   T3  PRISM lints, thread, interval, cppcheck, esbmc, pir, harness, conc: BOUNDED or better, none FAILED
#   T4  PRISM gateway stages on the harnesses: BOUNDED or better, none FAILED
# Tool flags and stage lists are fixed here or by environment on the worker, never by the agent.
# Exit 0 pass, 1 fail, 3 infrastructure error (tool missing, PRISM stage crashed). Results are printed
# as "PBSD-RESULT: <json>" (verdict per obligation) for the proof report.
#
# NOT YET RUN AGAINST A REAL PRISM BUILD. The stage lists and the report.json reading follow PRISM's
# source (include/prism/pipeline.hpp, src/prism/models.cpp); the first pilot step is to run this by
# hand on one P0-02 unit and fix whatever differs (AGENTS.md section 12).
set -uo pipefail
id="$1"; wt="$2"
wp="${id%%-U*}"; wp="${wp%-MODEL}"; wp="${wp%-INT}"; unit="$(sed -E 's/^.*-(U[0-9]+)-.*$/\1/' <<<"$id")"
PRISM_BIN="${PRISM_BIN:-prism}"
T="${PBSD_PROOF_TIMEOUT_S:-1800}"
LOCK="$wt/proofs/spec-lock.json"
fail() { echo "FAIL: $*"; exit 1; }
infra() { echo "infrastructure: $*"; exit 3; }
need() { command -v "$1" >/dev/null || infra "$1 not installed"; }

if [[ "$id" == *-INT ]]; then
  # Integrator: re-check every unit whose code or proofs the integration changed (tiers from the
  # pinned task list, never from the tree).
  tasks="${PBSD_TASKS:-/opt/pbsd/tasks.json}"; [ -f "$tasks" ] || infra "$tasks missing"
  base="$(git -C "$wt" log -1 --format=%B | sed -n 's/^PBSD-Base: //p')"; [ -n "$base" ] || infra "no PBSD-Base trailer"
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
[ -n "$tier" ] || infra "PBSD_PROOF_TIER not set by the runner"

# ---- banned constructs, everywhere the task can write proofs or annotations
banned_lean='(^|[^A-Za-z_])(sorry|admit|native_decide|implemented_by|extern)([^A-Za-z_]|$)|^[[:space:]]*axiom[[:space:]]|debug\.skipKernelTC|set_option[[:space:]]+(debug|trust)'
banned_c='__CPROVER_assume|__ESBMC_assume|__VERIFIER_assume|#pragma[[:space:]]+(CPROVER|ESBMC)'
scan() { grep -rnEI "$2" $1 2>/dev/null | grep -v '/harness/skeleton/' && fail "banned construct (see lines above)"; return 0; }

annotation_only() {   # the code with annotation comments removed must equal the reviewed code
  local base; base="$(git -C "$wt" log -1 --format=%B | sed -n 's/^PBSD-Base: //p')"
  [ -n "$base" ] || infra "no PBSD-Base trailer: cannot check the annotation-only diff"
  # PRISM annotations: `// requires: ...`, `// ensures: ...`, `// invariant: ...`, `// decreases: ...`
  # and single-line ACSL `/*@ ... */`.
  local strip='/^[[:space:]]*(\/\/[[:space:]]*(requires|ensures|invariant|decreases|assigns)[[:space:]]*:.*|\/\*@.*\*\/)[[:space:]]*$/d'
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
  m="$wt/proofs/$wp/model"
  [ -d "$m" ] || fail "no model in proofs/$wp/model"
  [ -f "$LOCK" ] || fail "proofs/spec-lock.json missing (written by P0-FREEZE)"
  # Locked statements unchanged since the freeze.
  need jq
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
  echo "PBSD-RESULT: [{\"unit\":\"$wp-model\",\"status\":\"PROVED\",\"tool\":\"lean/apalache\"}]"
  echo "PROVED: $wp model"; exit 0 ;;
T2|T3|T4)
  need jq; need "$PRISM_BIN"
  d="$wt/proofs/$wp/$unit"; [ -d "$d" ] || fail "no proofs/$wp/$unit"
  ob="$d/obligations.json"; [ -f "$ob" ] || fail "no obligations.json (the list of functions and properties to check)"
  [ "$(jq 'length' "$ob")" -gt 0 ] || fail "empty obligation list"
  scan "$d" "$banned_c"; scan "$wt/pbsd" "$banned_c"
  [ "$tier" = T4 ] || annotation_only
  # Scan only the directories that hold the obligations' files (the unit's code and harnesses).
  scan_dirs=(); while IFS= read -r f; do scan_dirs+=("$(dirname "$wt/$f")"); done < <(jq -r '.[].file' "$ob")
  scan_dirs+=("$d")
  work="$(mktemp -d "${TMPDIR:-/tmp}/prism-gate.XXXXXX")"; trap 'rm -rf "$work"' EXIT
  mkdir -p "$work/scope"; for p in $(printf '%s\n' "${scan_dirs[@]}" | sort -u); do
    rel="${p#$wt/}"; mkdir -p "$work/scope/$(dirname "$rel")"; cp -r "$p" "$work/scope/$rel"
  done
  case "$tier" in
    T2) stages="${PBSD_PRISM_STAGES_T2:-inventory,classify,lints,interval,esbmc,contracts,wp,pir,harness,review}" ;;
    T3) stages="${PBSD_PRISM_STAGES_T3:-inventory,classify,lints,taint,thread,interval,warnings,cppcheck,esbmc,pir,harness,conc}" ;;
    T4) stages="${PBSD_PRISM_STAGES_T4:-inventory,classify,lints,taint,interval,pir,harness}" ;;
  esac
  # No --allow-exec: nothing from the scanned tree is executed (PRISM Law 9). --fail-on defect makes
  # exit 1 mean "a FAILED/CRASH/SANFAIL finding"; exit 2 means a PRISM stage crashed (a PRISM bug).
  timeout "$T" "$PRISM_BIN" "$work/scope" --no-llm --jobs "${PBSD_JOBS:-2}" --stage "$stages" --out "$work/out" \
      --fail-on defect --unwind "${PBSD_PRISM_UNWIND:-32}" >"$work/prism.log" 2>&1
  rc=$?
  tail -25 "$work/prism.log"
  [ $rc -eq 124 ] && fail "PRISM timeout after ${T}s"
  [ $rc -eq 2 ] && infra "a PRISM stage crashed (see the log above)"
  rep="$work/out/report.json"; [ -f "$rep" ] || infra "PRISM wrote no report.json (exit $rc)"
  # A stage that could not run is infrastructure, never a pass (PRISM Law 1).
  jq -e '[.stages[] | select(.name as $n | ("'"$stages"'" | split(",") | index($n))) | select(.status=="NOTRUN")] | length == 0' "$rep" >/dev/null \
    || { jq -r '.stages[] | select(.status=="NOTRUN") | "NOTRUN stage \(.name): \(.detail)"' "$rep" | head -10; infra "a required PRISM stage did not run (tool missing?)"; }
  [ $rc -eq 1 ] && { jq -r '.stages[].findings[] | select(.status=="FAILED" or .status=="CRASH" or .status=="SANFAIL") | "\(.status) \(.stage) \(.file):\(.line // 0) \(.function // "") \(.cls): \(.message)"' "$rep" | head -20; fail "PRISM found a defect"; }
  results="[]"
  while IFS=$'\t' read -r file fn; do
    best="$(jq -r --arg f "$file" --arg fn "$fn" '
        [.stages[].findings[] | select(.function==$fn and (.file|endswith($f)))] as $x
        | if ($x|length)==0 then "NOFINDING"
          elif ([$x[] | select(.status=="VACUOUS-ASSUMPTION" or (.cls|tostring|test("VACUOUS")))]|length)>0 then "VACUOUS"
          elif ([$x[] | select(.status|IN("FAILED","CRASH","SANFAIL","ERROR"))]|length)>0 then "FAILED"
          else ([$x[].status] | map(select(IN("PROVED-CERTIFIED","PROVED-UNBOUNDED","PROVED","PROVED-ASSUMING","BOUNDED"))) |
                (if index("PROVED-CERTIFIED") then "PROVED-CERTIFIED" elif index("PROVED-UNBOUNDED") then "PROVED-UNBOUNDED"
                 elif index("PROVED") then "PROVED" elif index("PROVED-ASSUMING") then "PROVED-ASSUMING"
                 elif index("BOUNDED") then "BOUNDED" else "NOANSWER" end)) end' "$rep")"
    case "$tier:$best" in
      T2:PROVED-CERTIFIED|T2:PROVED-UNBOUNDED|T2:PROVED|T2:PROVED-ASSUMING) ;;
      T3:*PROVED*|T3:BOUNDED|T4:*PROVED*|T4:BOUNDED) ;;
      *) jq -r --arg f "$file" --arg fn "$fn" '.stages[].findings[] | select(.function==$fn and (.file|endswith($f))) | "  \(.stage) \(.status) \(.cls): \(.message)"' "$rep" | head -8
         fail "$fn: $best (needs $( [ "$tier" = T2 ] && echo 'a PROVED verdict' || echo 'BOUNDED or better'))" ;;
    esac
    results="$(jq -c --arg f "$file" --arg fn "$fn" --arg s "$best" '. + [{file:$f, function:$fn, status:$s, tool:"prism"}]' <<<"$results")"
  done < <(jq -r '.[] | [.file, .function] | @tsv' "$ob")
  echo "PBSD-RESULT: $results"
  echo "$tier: $(jq length <<<"$results") obligation(s) checked by PRISM"; exit 0 ;;
*) fail "unknown tier $tier" ;;
esac
