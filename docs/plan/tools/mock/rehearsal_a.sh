#!/usr/bin/env bash
# Rehearsal, phase A: the P0-02 slice (IF, three units of SPEC/IMPL/TEST/PROOF/REVIEW, INT) through the REAL runner, gates and Morph API,
# isolated from the pilot's state and repository. Dependencies on tasks outside the slice are trimmed. Phase B (the T1 model) is rehearsal_b.sh.
#   tools/mock/rehearsal_a.sh [spend-limit-usd]       default 8; the runner has no cost cap of its own, so this script stops it
LIMIT="${1:-8}"
. ~/.pbsd/env
D=/run/pbsd/reh; rm -rf $D; mkdir -p $D/state $D/wt $D/ckpt
git clone -q --local --no-checkout /run/pbsd/repo $D/repo || exit 1
git -C $D/repo sparse-checkout set docs/plan hbsd kde
git -C $D/repo checkout -q -B pbsd-cutover "$(git -C /run/pbsd/repo rev-parse pbsd-cutover)"
echo "repo: $(git -C $D/repo log --oneline -1 | cut -c1-70)"
jq '[.[] | select(.id|test("^P0-02-(IF|INT)$|^P0-02-U00[123]-"))] as $s | ($s|map(.id)) as $ids
    | $s | map(.deps |= map(select(. as $d | $ids|index($d))))' /opt/pbsd/tasks.json > $D/tasks.json
echo "tasks in phase A: $(jq length $D/tasks.json)"
jq --arg d "$D" '.max_attempts = 3 | .slots.agent.count = 4 | .checkpoint_dir = ($d + "/ckpt") | .checkpoint_repo = ($d + "/ckpt/pbsd.git")' ~/.pbsd/config.json > $D/config.json
export PBSD_REPO=$D/repo PBSD_STATE=$D/state PBSD_WT=$D/wt PBSD_CHECKPOINT=$D/ckpt PBSD_TASKS=$D/tasks.json PBSD_CONFIG=$D/config.json
P=$HOME/pbsd-run-build/pbsd_run
cd $D/repo; $P init 2>&1 | tail -1
s=$(date +%s)
timeout 10800 $P run > $D/run.log 2>&1 &
RP=$!
# spend watchdog: stop the runner if the API cost passes the limit
( while kill -0 $RP 2>/dev/null; do
    c=$($P status 2>/dev/null | grep -o 'API cost so far \$[0-9.]*' | grep -o '[0-9.]*$' | head -1)
    if [ -n "$c" ] && awk -v c="$c" -v l="$LIMIT" 'BEGIN{exit !(c>l)}'; then echo "SPEND LIMIT \$$LIMIT passed (\$$c): stopping the runner" >> $D/watchdog.log; kill $RP; break; fi
    sleep 30
  done ) &
wait $RP; echo "runner exit $? after $(( $(date +%s) - s )) s"
echo "== status"; $P status 2>&1 | head -6 | cut -c1-150
echo "== per task"
sqlite3 -separator '  ' $D/state/tasks.db "select id, state, attempts from task order by id" 2>&1 | head -30
echo "== cost by role"
sqlite3 -separator '  ' $D/state/tasks.db "select t.role, count(distinct c.task), round(sum(c.usd),3) from cost c join task t on t.id=c.task group by 1" 2>&1
[ -f $D/watchdog.log ] && cat $D/watchdog.log
