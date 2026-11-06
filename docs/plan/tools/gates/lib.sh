#!/usr/bin/env bash
# Shared gate steps. Every step is a hook so the real PBSD build, proof and QEMU commands can be set
# per machine without editing the scripts (P0-07 fixes them):
#   PBSD_BUILD_CMD  build the tree out of tree in $PBSD_BUILD_DIR  (default: CMake + Ninja on pbsd/)
#   PBSD_TEST_CMD   unit + fault-injection tests                   (default: ctest in $PBSD_BUILD_DIR)
#   PBSD_PROOF_CMD  proof check                                    (default: proof_check.sh, tier from the task)
#   PBSD_QEMU_CMD   boot test                                      (default: qemu_boot.sh)
# The gate scripts are pinned copies (/opt/pbsd/gates, or the runner's gates_dir), never the copy in
# the task's commit. Exit codes: 0 pass, 1 gate failed, 3 infrastructure error (retried elsewhere).
set -uo pipefail
GATE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PBSD_JOBS="${PBSD_JOBS:-2}"           # each gate slot owns 2 cores; never -j$(nproc)
log() { printf '[gate %s] %s\n' "$(date +%H:%M:%S)" "$*"; }
step() {  # step <name> <timeout-seconds> <command...>
  local name="$1" to="$2"; shift 2
  log "begin $name"
  timeout --kill-after=30 "$to" bash -c "$*"; local rc=$?
  if [ $rc -eq 124 ] || [ $rc -eq 137 ]; then log "FAIL $name: watchdog timeout after ${to}s"; return 1; fi
  if [ $rc -eq 3 ]; then log "INFRA $name"; return 3; fi
  if [ $rc -ne 0 ]; then log "FAIL $name (exit $rc)"; return 1; fi
  log "pass $name"
}
bdir() { echo "${PBSD_BUILD_DIR:-$1/.build}"; }
do_build() {
  local b; b="$(bdir "$1")"; mkdir -p "$b"
  step build "${PBSD_BUILD_TIMEOUT:-3600}" "${PBSD_BUILD_CMD:-cmake -S \"$1/pbsd\" -B \"$b\" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPBSD_FAULT_INJECTION=ON >/dev/null && ninja -C \"$b\" -j$PBSD_JOBS}"
}
do_test() {
  local b; b="$(bdir "$1")"
  if [ -n "${PBSD_TEST_CMD:-}" ]; then step test "${PBSD_TEST_TIMEOUT:-3600}" "$PBSD_TEST_CMD"
  else step test "${PBSD_TEST_TIMEOUT:-3600}" "ctest --test-dir \"$b\" -j$PBSD_JOBS --output-on-failure --no-tests=error"; fi
}
do_proof() { step proof "${PBSD_PROOF_TIMEOUT:-7200}" "${PBSD_PROOF_CMD:-\"$GATE_DIR/proof_check.sh\" \"$GATE_ID\" \"$1\"}"; }
do_qemu()  { step qemu  "${PBSD_QEMU_TIMEOUT:-1800}" "${PBSD_QEMU_CMD:-\"$GATE_DIR/qemu_boot.sh\" \"$1\"}"; }
gate_main() {  # gate_main <task-id> <worktree> <steps...>
  local id="$1" wt="$2"; shift 2
  export GATE_ID="$id" GATE_WT="$wt"   # hooks (PBSD_BUILD_CMD etc.) find the worktree here
  [ -d "$wt" ] || { log "worktree $wt missing"; exit 3; }
  log "task $id on $(hostname) in $wt: $*"
  for s in "$@"; do "do_$s" "$wt"; rc=$?; [ $rc -eq 0 ] || exit $rc; done
  log "GATE PASS $id"
}
