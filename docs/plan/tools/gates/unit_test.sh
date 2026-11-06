#!/usr/bin/env bash
# Interim PBSD_TEST_CMD: build and run the test files THIS task owns (each tests/**/*.cpp is a program with its own main()).
# A -TEST task must own at least one; every other role passes with nothing to run. The differential run against the Linux/HBSD
# original (AGENTS.md section 1) is not done here: P0-07 and the per-package harnesses add it.
#   PBSD_TEST_CMD=/opt/pbsd/gates/unit_test.sh     (the gate sets GATE_ID and GATE_WT; PBSD_TASKS is the pinned task list)
set -uo pipefail
id="${GATE_ID:?GATE_ID not set (run through lib.sh)}"; wt="${GATE_WT:?GATE_WT not set}"
tasks="${PBSD_TASKS:-/opt/pbsd/tasks.json}"; [ -f "$tasks" ] || { echo "infrastructure: $tasks missing"; exit 3; }
command -v jq >/dev/null || { echo "infrastructure: jq missing"; exit 3; }
cxx="${PBSD_CXX:-clang++}"; command -v "$cxx" >/dev/null || { echo "infrastructure: $cxx missing"; exit 3; }
cd "$wt" || exit 3
role="$(jq -r --arg id "$id" '.[] | select(.id==$id) | .role' "$tasks")"
mapfile -t owns < <(jq -r --arg id "$id" '.[] | select(.id==$id) | .owns[]?' "$tasks")
inc=(); for d in pbsd/kernel/include pbsd/kernel/base pbsd/include tests; do [ -d "$d" ] && inc+=("-I$d"); done
bin="$(mktemp -d)"; trap 'rm -rf "$bin"' EXIT
n=0; fail=0
for o in "${owns[@]}"; do
  case "$o" in tests/*) ;; *) continue ;; esac
  [ -d "$o" ] || [ -f "$o" ] || continue
  while IFS= read -r f; do
    n=$((n+1)); exe="$bin/t$n"
    if "$cxx" -std=c++23 -Wall -Wextra "${inc[@]}" "$f" -o "$exe" 2> "$bin/err"; then
      if ! timeout "${PBSD_UNIT_TEST_TIMEOUT:-120}" "$exe" > "$bin/out" 2>&1; then echo "FAIL (exit $?): $f"; tail -20 "$bin/out"; fail=$((fail+1)); fi
    else echo "FAIL (does not compile): $f"; head -30 "$bin/err"; fail=$((fail+1)); fi
  done < <(find "$o" -type f \( -name '*.cpp' -o -name '*.cc' \) -not -path '*/.lake/*' | sort)
done
if [ "$n" -eq 0 ] && [ "$role" = "tester" ]; then echo "FAIL: a tester task must write at least one tests/**/*.cpp program"; exit 1; fi
echo "unit_test: $n test program(s) run for $id ($role), $fail failed"
[ "$fail" -eq 0 ]
