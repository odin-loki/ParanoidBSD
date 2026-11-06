#!/usr/bin/env bash
# Interim PBSD_TEST_CMD: build and run the tests THIS task owns.
# Convention (told to testers in AGENTS.md): every directory under tests/ that holds .cpp files is ONE program, built from all of its .cpp
# files together, with exactly one main(); local headers sit next to the sources. If that does not link (for example several independent
# files each with their own main), each file is tried on its own instead. A -TEST task must own at least one program; every other role
# passes with nothing to run. The differential run against the Linux/HBSD original (AGENTS.md section 1) is not done here: P0-07 adds it.
# First rehearsal (6 Nov 2026): the earlier version built each file as its own program, which rejected a normal driver + helper layout, and
# it printed "exit 0" for a failing program (the exit code was read after the `!`).
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
n=0; fail=0; limit="${PBSD_UNIT_TEST_TIMEOUT:-120}"

run_one() {   # run_one <label> <source files...>: compile+link, run; returns 0 ran (result counted), 1 did not build
  local label="$1"; shift; local exe="$bin/t$((n+1))" rc
  if ! "$cxx" -std=c++23 -Wall -Wextra "${inc[@]}" -I"$(dirname "$1")" "$@" -o "$exe" 2> "$bin/err"; then return 1; fi
  n=$((n+1))
  timeout "$limit" "$exe" > "$bin/out" 2>&1; rc=$?
  if [ "$rc" -ne 0 ]; then echo "FAIL (exit $rc): $label"; tail -25 "$bin/out"; fail=$((fail+1)); fi
  return 0
}

declare -A seen=()
for o in "${owns[@]}"; do
  o="${o%.\*}"; case "$o" in tests/*) ;; *) continue ;; esac
  [ -e "$o" ] || continue
  while IFS= read -r f; do seen["$(dirname "$f")"]=1; done < <(find "$o" -type f \( -name '*.cpp' -o -name '*.cc' \) -not -path '*/.lake/*' | sort)
done
for d in $(printf '%s\n' "${!seen[@]}" | sort); do
  mapfile -t srcs < <(find "$d" -maxdepth 1 -type f \( -name '*.cpp' -o -name '*.cc' \) | sort)
  [ "${#srcs[@]}" -gt 0 ] || continue
  if run_one "$d (${#srcs[@]} files)" "${srcs[@]}"; then continue; fi
  combined_err="$(head -30 "$bin/err")"
  built_any=0
  if [ "${#srcs[@]}" -gt 1 ]; then for s in "${srcs[@]}"; do run_one "$s" "$s" && built_any=1; done; fi
  if [ "$built_any" -eq 0 ]; then echo "FAIL (does not build): $d"; echo "$combined_err"; fail=$((fail+1)); n=$((n+1)); fi
done
if [ "$n" -eq 0 ] && [ "$role" = "tester" ]; then echo "FAIL: a tester task must write at least one test program under tests/<WP>/<U>/ (all .cpp files of a directory are built together, one main())"; exit 1; fi
echo "unit_test: $n test program(s) for $id ($role), $fail failed"
[ "$fail" -eq 0 ]
