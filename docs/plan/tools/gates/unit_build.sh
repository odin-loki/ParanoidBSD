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
for o in "${owns[@]}"; do
  o="${o%.\*}"   # "dir/name.*" means every file of that stem
  if [ -f "$o" ]; then check "$o"
  elif [ -d "$o" ]; then
    while IFS= read -r f; do check "$f"; done < <(find "$o" -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' -o -name '*.hpp' -o -name '*.h' -o -name '*.hh' \) -not -path '*/.lake/*' -not -path '*/proofs/*' | sort)
  else
    for f in "$o".*; do [ -f "$f" ] && check "$f"; done
  fi
done
echo "unit_build: $n C++ file(s) checked for $id, $fail failed"
[ "$fail" -eq 0 ]
