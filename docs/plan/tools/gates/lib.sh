#!/usr/bin/env bash
# Shared gate steps. Every step is a hook so the real PBSD build, PRISM and QEMU commands can be
# set per machine without editing the scripts:
#   PBSD_BUILD_CMD  build the tree        (default: make -j$PBSD_JOBS)
#   PBSD_TEST_CMD   unit + fault-injection tests (default: make -j$PBSD_JOBS check, if a check target exists)
#   PBSD_PROOF_CMD  PRISM proof check     (default: tools/gates/prism_check.sh)
#   PBSD_QEMU_CMD   boot test             (default: tools/gates/qemu_boot.sh)
# Exit codes: 0 pass, 1 gate failed, 3 infrastructure error (runner retries elsewhere).
set -uo pipefail
GATE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PBSD_JOBS="${PBSD_JOBS:-2}"           # each gate slot owns 2 cores; never -j$(nproc)
log() { printf '[gate %s] %s\n' "$(date +%H:%M:%S)" "$*"; }
step() {  # step <name> <timeout-seconds> <command...>
  local name="$1" to="$2"; shift 2
  log "begin $name"
  timeout --kill-after=30 "$to" bash -c "$*"; local rc=$?
  if [ $rc -eq 124 ] || [ $rc -eq 137 ]; then log "FAIL $name: watchdog timeout after ${to}s"; return 1; fi
  if [ $rc -ne 0 ]; then log "FAIL $name (exit $rc)"; return 1; fi
  log "pass $name"
}
has_check_target() { make -C "$1" -n check >/dev/null 2>&1; }
do_build() { step build "${PBSD_BUILD_TIMEOUT:-3600}" "${PBSD_BUILD_CMD:-make -C \"$1\" -j$PBSD_JOBS}"; }
do_test() {
  if [ -n "${PBSD_TEST_CMD:-}" ]; then step test "${PBSD_TEST_TIMEOUT:-3600}" "$PBSD_TEST_CMD"
  elif has_check_target "$1"; then step test "${PBSD_TEST_TIMEOUT:-3600}" "make -C \"$1\" -j$PBSD_JOBS check"
  else log "skip test: no check target"; fi
}
do_proof() { step proof "${PBSD_PROOF_TIMEOUT:-3600}" "${PBSD_PROOF_CMD:-\"$GATE_DIR/prism_check.sh\" \"$1\"}"; }
do_qemu()  { step qemu  "${PBSD_QEMU_TIMEOUT:-900}"  "${PBSD_QEMU_CMD:-\"$GATE_DIR/qemu_boot.sh\" \"$1\"}"; }
gate_main() {  # gate_main <task-id> <worktree> <steps...>
  local id="$1" wt="$2"; shift 2
  [ -d "$wt" ] || { log "worktree $wt missing"; exit 3; }
  log "task $id on $(hostname) in $wt: $*"
  for s in "$@"; do "do_$s" "$wt" || exit 1; done
  log "GATE PASS $id"
}
