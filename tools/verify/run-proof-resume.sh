#!/usr/bin/env bash
# Resume after SIGTERM: goto binaries exist, ESBMC and analyze already ran.
set -u
export PYTHONUNBUFFERED=1
export PATH="${HOME}/.local/bin:/usr/bin:/bin:${PATH}"
OUT=/home/odin/pbsd-sweep
LOG="$OUT/campaign.log"
exec >>"$LOG" 2>&1
echo "== $(date -Is) proof resume (cbmc-retry, pointer a, pointer b)"

overall=0
echo "== $(date -Is) START cbmc-retry"
/home/odin/run-cbmc-retry.sh || overall=$?
echo "== $(date -Is) END cbmc-retry exit=$overall"

echo "== $(date -Is) START pointer-null-depth"
/home/odin/run-pointer-cbmc.sh a || overall=$?
echo "== $(date -Is) END pointer-null-depth exit=$?"

echo "== $(date -Is) START pointer-harness"
/home/odin/run-pointer-cbmc.sh b || overall=$?
echo "== $(date -Is) END pointer-harness exit=$?"

echo "== $(date -Is) proof resume end overall_exit=$overall"
exit "$overall"
