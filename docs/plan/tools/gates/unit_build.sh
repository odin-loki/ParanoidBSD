#!/usr/bin/env bash
# Interim PBSD_BUILD_CMD: compile only what THIS task owns, not the whole pbsd/ tree.
# Why: the default gate builds all of pbsd/, which does not build today (about 157 modules cannot find headers that live in
# hbsd/src/, excluded from the gate's sparse worktree), so every build gate would fail whatever the agent wrote. P0-07 replaces
# this with the real build; until then this checks that the owned C++ is syntactically and semantically valid (-fsyntax-only).
#   PBSD_BUILD_CMD=/opt/pbsd/gates/unit_build.sh     (the gate sets GATE_ID and GATE_WT; PBSD_TASKS is the pinned task list)
set -uo pipefail
id="${GATE_ID:?GATE_ID not set (run through lib.sh)}"; wt="${GATE_WT:?GATE_WT not set}"
tasks="${PBSD_TASKS:-/opt/pbsd/tasks.json}"; [ -f "$tasks" ] || { echo "infrastructure: $tasks missing"; exit 3; }
command -v jq >/dev/null || { echo "infrastructure: jq missing"; exit 3; }
cxx="${PBSD_CXX:-clang++}"; command -v "$cxx" >/dev/null || { echo "infrastructure: $cxx missing"; exit 3; }
cd "$wt" || exit 3
mapfile -t owns < <(jq -r --arg id "$id" '.[] | select(.id==$id) | .owns[]?' "$tasks")
inc=(); for d in pbsd/kernel/include pbsd/kernel/base pbsd/include; do [ -d "$d" ] && inc+=("-I$d"); done
n=0; fail=0
check() {  # check <file>
  local f="$1" tu
  case "$f" in
    *.cpp|*.cc|*.cxx) tu="$f" ;;
    *.hpp|*.h|*.hh) tu="$(mktemp --suffix=.cpp)"; printf '#include "%s/%s"\n' "$wt" "$f" > "$tu" ;;   # a header must stand alone
    *) return 0 ;;
  esac
  n=$((n+1))
  if ! "$cxx" -std=c++23 -fsyntax-only -Wall -Wextra "${inc[@]}" "$tu" 2> /tmp/unit_build.$$; then
    echo "FAIL: $f"; head -30 /tmp/unit_build.$$; fail=$((fail+1))
  fi
  rm -f /tmp/unit_build.$$; [ "$tu" != "$f" ] && rm -f "$tu"
  return 0
}
# Content checks. A syntax check alone accepts a file that holds only a comment, and in the first rehearsal (6 Nov 2026) a decomposer
# "passed" by writing `/* ${doc} */` into three files (a spec, a Lean statement file and a header). Reject placeholders and empty files.
stub=0
# Only files THIS task changed are checked: an owned directory may hold older files (a comment-only header from an earlier task, say) that
# are not this task's doing and must not block it. The runner puts the base commit in the gate commit's PBSD-Base trailer.
base="$(git log -1 --format=%B 2>/dev/null | sed -n 's/^PBSD-Base: //p' | head -1)"
changed=(); [ -n "$base" ] && mapfile -t changed < <(git diff --name-only "$base" HEAD 2>/dev/null)
in_changed() { [ -z "$base" ] && return 0; printf '%s\n' "${changed[@]}" | grep -qxF -- "$1"; }
content_check() {  # content_check <file>
  local f="$1" stripped lines
  [ -f "$f" ] || return 0
  in_changed "$f" || return 0
  case "$f" in *.sh|*.json|*.toml|*.lock|*/.lake/*) return 0 ;; esac
  if grep -Eq '\$\{[A-Za-z_][A-Za-z0-9_]*\}|\{\{[a-z_]+\}\}|<PLACEHOLDER>|TODO: write' "$f" 2>/dev/null; then echo "FAIL: $f contains a template placeholder instead of content"; stub=$((stub+1)); return 0; fi
  case "$f" in
    *.h|*.hpp|*.hh|*.cpp|*.cc|*.cxx|*.cppm)
      stripped="$(sed -E ':a;N;$!ba;s#/\*([^*]|\*+[^*/])*\*+/##g;s#//[^\n]*##g' "$f" | tr -d '[:space:]')"
      if [ "${#stripped}" -lt 30 ]; then echo "FAIL: $f has no code (only comments or fewer than 30 characters)"; stub=$((stub+1)); fi ;;
    *.md)
      lines="$(grep -c '[^[:space:]]' "$f")"
      if [ "$lines" -lt 12 ]; then echo "FAIL: $f has only $lines non-blank lines; a spec needs real content"; stub=$((stub+1)); fi ;;
    *.lean)
      if ! grep -Eq '^[[:space:]]*(theorem|lemma|def|example|inductive|structure|class|instance|abbrev)\b' "$f"; then echo "FAIL: $f declares nothing (no theorem, def, structure ...)"; stub=$((stub+1)); fi ;;
  esac
}
for o in "${owns[@]}"; do
  o="${o%.\*}"   # "dir/name.*" means every file of that stem
  if [ -f "$o" ]; then content_check "$o"
  elif [ -d "$o" ]; then while IFS= read -r f; do content_check "$f"; done < <(find "$o" -type f -not -path '*/.lake/*' -not -path '*/.git/*' | sort)
  else for f in "$o".*; do content_check "$f"; done; fi
done
for o in "${owns[@]}"; do
  o="${o%.\*}"   # "dir/name.*" means every file of that stem
  if [ -f "$o" ]; then check "$o"
  elif [ -d "$o" ]; then
    while IFS= read -r f; do check "$f"; done < <(find "$o" -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' -o -name '*.hpp' -o -name '*.h' -o -name '*.hh' \) -not -path '*/.lake/*' -not -path '*/proofs/*' | sort)
  else
    for f in "$o".*; do [ -f "$f" ] && check "$f"; done
  fi
done
# Lean statement files must COMPILE. In the second rehearsal (6 Nov 2026) the decomposer's proofs/P0-02/spec/Spec.lean passed this gate and was
# frozen although `deriving DecidableEq, Repr` on a structure with function fields does not compile, so the model task could prove nothing.
# The spec directory is built as the Spec library of the pinned Lake template (Lean 4.9 + Mathlib, shared packages linked, not fetched).
spec_fail=0
mapfile -t spec_dirs < <(
  if [ -n "$base" ]; then printf '%s\n' "${changed[@]}"; else find proofs -path '*/spec/*.lean' -type f 2>/dev/null; fi |
    grep -E '^proofs/[^/]+/spec/.*\.lean$' | xargs -r -n1 dirname | sort -u)
if [ "${#spec_dirs[@]}" -gt 0 ]; then
  tmpl="${PBSD_TEMPLATES:-/opt/pbsd/templates}/model-lake"
  if ! command -v lake >/dev/null || [ ! -d "$tmpl" ] || [ ! -d "${PBSD_LEAN_PACKAGES:-/nonexistent}" ]; then
    echo "infrastructure: lake, $tmpl or PBSD_LEAN_PACKAGES missing: cannot compile the Lean spec"; exit 3
  fi
  for sd in "${spec_dirs[@]}"; do
    tmp="$(mktemp -d)"; mkdir -p "$tmp/model/.lake" "$tmp/spec"
    cp -r "$tmpl"/. "$tmp/model/"; cp -r "$sd"/. "$tmp/spec/"
    ln -s "$PBSD_LEAN_PACKAGES" "$tmp/model/.lake/packages"
    if ! (cd "$tmp/model" && timeout "${PBSD_LEAN_SPEC_TIMEOUT:-600}" lake build Spec > "$tmp/build.log" 2>&1); then
      echo "FAIL: $sd does not compile as the Spec library (Lean 4.9, Mathlib):"; grep -v '^\(info\|trace\|✔\)' "$tmp/build.log" | head -25
      spec_fail=$((spec_fail+1))
    fi
    rm -rf "$tmp"
  done
fi
echo "unit_build: $n C++ file(s) checked for $id, $fail failed, $stub placeholder/empty file(s), $spec_fail Lean spec dir(s) that do not compile"
[ "$fail" -eq 0 ] && [ "$stub" -eq 0 ] && [ "$spec_fail" -eq 0 ]
