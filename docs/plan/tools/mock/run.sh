#!/usr/bin/env bash
# Runs the mock pilot: model server, runner init + run, then a status report. Usage: run.sh [comma-separated IMPL ids to break once]
M=/tmp/mock; HERE="$(cd "$(dirname "$0")" && pwd)"
export PBSD_STATE=$M/state PBSD_WT=$M/wt PBSD_REPO=$M/repo PBSD_CHECKPOINT=$M/ckpt
export PBSD_TASKS=$M/tasks.json PBSD_CONFIG=$M/config.json MOCK_KEY=x
export PBSD_BUILD_CMD=$M/build.sh PBSD_TEST_CMD=$M/test.sh PBSD_QEMU_CMD=$M/qemu.sh
export PBSD_JOBS=4 PRISM_BIN=$HOME/PRISM/build/prism PRISM_TOOLS_DIR=$HOME/.prism/tools
export PBSD_PRISM_UNWIND=8 PBSD_PROOF_TIMEOUT_S=300
python3 $M/mock_model.py 18080 "${1:-}" > $M/model.out 2>&1 &
sleep 1
cd $M/repo
P=$HOME/pbsd-run-build/pbsd_run
$P init 2>&1 | tail -2
timeout "${RUN_TIMEOUT:-420}" $P run > $M/run.log 2>&1
echo "run exit $?"
$P status 2>&1 | head -12 | cut -c1-200
pkill -f mock_model.py
