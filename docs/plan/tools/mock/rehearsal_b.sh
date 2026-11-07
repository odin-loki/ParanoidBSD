#!/usr/bin/env bash
# Rehearsal, phase B: P0-02-IF for real (writes the contract headers and the locked Lean statements), then a freeze stand-in (hashes the
# statements into proofs/spec-lock.json, which P0-FREEZE does in the real graph), then P0-02-MODEL (tier T1: the Goedel prover through
# lean_prove, the Lake template, Mathlib) -- real runner, real gates, real Morph. Isolated from the pilot.
#   tools/mock/rehearsal_b.sh [spend-limit-usd]       default 3
LIMIT="${1:-3}"
pkill -f "prism prove" 2>/dev/null; sleep 1   # a prover left over from an earlier run would hold the GPU (7 Nov 2026)
. ~/.pbsd/env
D=/run/pbsd/rehb; rm -rf $D; mkdir -p $D
git clone -q --local --no-checkout /run/pbsd/repo $D/repo || exit 1
git -C $D/repo sparse-checkout set docs/plan hbsd kde
git -C $D/repo checkout -q -B pbsd-cutover "$(git -C /run/pbsd/repo rev-parse pbsd-cutover)"
echo "repo: $(git -C $D/repo log --oneline -1 | cut -c1-70)"
P=$HOME/pbsd-run-build/pbsd_run
spend_watchdog() {  # spend_watchdog <runner pid>
  while kill -0 "$1" 2>/dev/null; do
    c=$($P status 2>/dev/null | grep -o 'API cost so far \$[0-9.]*' | grep -o '[0-9.]*$' | head -1)
    if [ -n "$c" ] && awk -v c="$c" -v l="$LIMIT" 'BEGIN{exit !(c>l)}'; then echo "SPEND LIMIT \$$LIMIT passed (\$$c): stopping" >> $D/watchdog.log; kill "$1"; break; fi
    sleep 30
  done
}
phase() {  # phase <name> <task-id-regex> <timeout-s>
  local name="$1" re="$2" to="$3" S=$D/state-$1
  mkdir -p $S $D/wt-$1 $D/ckpt-$1
  jq --arg re "$re" '[.[] | select(.id|test($re))] as $s | ($s|map(.id)) as $ids | $s | map(.deps |= map(select(. as $d | $ids|index($d))))' /opt/pbsd/tasks.json > $D/tasks-$name.json
  jq --arg d "$D" --arg n "$name" '.max_attempts = 3 | .slots.agent.count = 2 | .checkpoint_dir = ($d + "/ckpt-" + $n) | .checkpoint_repo = ($d + "/ckpt-" + $n + "/pbsd.git")' ~/.pbsd/config.json > $D/config-$name.json
  export PBSD_REPO=$D/repo PBSD_STATE=$S PBSD_WT=$D/wt-$name PBSD_CHECKPOINT=$D/ckpt-$name PBSD_TASKS=$D/tasks-$name.json PBSD_CONFIG=$D/config-$name.json
  ( cd $D/repo && $P init 2>&1 | tail -1 )
  local s=$(date +%s)
  ( cd $D/repo && timeout "$to" $P run > $D/run-$name.log 2>&1 ) & local rp=$!
  spend_watchdog $rp & local wp=$!
  wait $rp; kill $wp 2>/dev/null
  echo "== phase $name: ended after $(( $(date +%s) - s )) s"
  sqlite3 -separator '  ' $S/tasks.db "select id,state,attempts from task order by id"
  sqlite3 -separator '  ' $S/tasks.db "select 'cost USD', round(sum(usd),3) from cost"
}

phase if '^P0-02-IF$' 3000
echo "== what IF wrote (spec statements):"; git -C $D/repo show pbsd-cutover:proofs/P0-02/spec/Spec.lean 2>&1 | head -40 | cut -c1-150

# freeze stand-in: hash the spec files and list the theorem names (the `def name : Prop` lines inside namespace Spec)
cd $D/repo
specs=$(git ls-tree -r --name-only pbsd-cutover -- proofs/P0-02/spec)
[ -n "$specs" ] || { echo "no spec files: IF did not write proofs/P0-02/spec"; exit 1; }
names=$(git show pbsd-cutover:proofs/P0-02/spec/Spec.lean | sed -n 's/^[[:space:]]*def[[:space:]]\+\([A-Za-z_][A-Za-z0-9_'"'"']*\)[[:space:]]*:[[:space:]]*Prop.*/\1/p' | jq -R . | jq -s .)
echo "theorem names found: $names"
mkdir -p proofs/P0-02/spec; git show pbsd-cutover:proofs/P0-02/spec/Spec.lean > proofs/P0-02/spec/Spec.lean
h=$(sha256sum proofs/P0-02/spec/Spec.lean | cut -d' ' -f1)
jq -n --arg h "$h" --argjson n "$names" '{"proofs/P0-02/spec/Spec.lean": $h, theorems: {"P0-02": $n}}' > proofs/spec-lock.json
git add --sparse proofs/spec-lock.json proofs/P0-02/spec/Spec.lean
git -c user.name=freeze -c user.email=f@f commit -q -m "freeze stand-in: hash the P0-02 spec (what P0-FREEZE does in the real graph)" && echo "froze: $(git log --oneline -1 | cut -c1-70)"
[ "$(echo "$names" | jq length)" -gt 0 ] || { echo "no 'def name : Prop' statements in Spec.lean: stopping before the T1 task"; exit 1; }

phase model '^P0-02-MODEL$' 5400
echo "== what MODEL wrote:"; git -C $D/repo ls-tree -r --name-only pbsd-cutover -- proofs/P0-02/model | head -12
git -C $D/repo show pbsd-cutover:proofs/P0-02/model/Model.lean 2>&1 | head -40 | cut -c1-150
[ -f $D/watchdog.log ] && cat $D/watchdog.log
