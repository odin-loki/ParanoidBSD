#!/usr/bin/env bash
# Builds /tmp/mock: a scratch repo, a mini task list from the real P0-02 tasks, a config pointing at the mock model, mock gate hooks.
set -e
HERE0="$(cd "$(dirname "$0")" && pwd)"; SRC="${PBSD_SRC:-$(cd "$HERE0/../../../.." && pwd)}"
HERE="$(cd "$(dirname "$0")" && pwd)"
M=/tmp/mock
pkill -f mock_model.py || true
rm -rf $M; mkdir -p $M/state $M/wt $M/ckpt $M/gates $M/repo
cp $SRC/docs/plan/tools/gates/*.sh $M/gates/
cp $HERE/mock_model.py $M/
cd $M/repo
git init -q -b pbsd-cutover
git config user.email m@m; git config user.name mock
mkdir -p docs/plan pbsd/kernel/include/cap
cp $SRC/docs/plan/AGENTS.md docs/plan/
echo mock > README.md
echo "// base" > pbsd/kernel/include/cap/base.hpp
git add -A; git commit -q -m base
git branch pbsd-base
jq '[.[] | select(.id|test("^P0-02-(IF|INT)$|^P0-02-U00[123]-"))] as $s | ($s|map(.id)) as $ids
    | $s | map(.deps |= map(select(. as $d | $ids|index($d)))
              | if .id=="P0-02-U001-PROOF" then .proof_tier="T2"
                elif .id=="P0-02-U002-PROOF" then .proof_tier="T3"
                elif .id=="P0-02-U003-PROOF" then .proof_tier="T4" else . end)' \
  $SRC/docs/plan/tasks.json > $M/tasks.json
echo "tasks: $(jq length $M/tasks.json)"
jq --arg gates $M/gates '.providers=[{"name":"mock","tier":"flash","weight":1,"url":"http://127.0.0.1:18080/v1/chat/completions","model":"mock","key_env":"MOCK_KEY","concurrency":4,"rpm":0,"tpm":0,"price_in":0.047,"price_hit":0.02,"price_out":0.268}]
    | .gates_dir=$gates | .slots.agent.count=4 | .slots.compile.count=2 | .slots.proof.count=2 | .slots.qemu.count=1
    | .checkpoint_dir="/tmp/mock/ckpt" | .checkpoint_repo="/tmp/mock/ckpt/pbsd.git" | .checkpoint_s=60
    | .ref_roots={"linux":"/tmp/mock/none","wine":"/tmp/mock/none","hbsd":"/tmp/mock/repo/hbsd","kde":"/tmp/mock/repo/kde"}
    | .prism_bin="$HOME/PRISM/build/prism"' \
  $SRC/docs/plan/tools/runner/config.glm-api.json > $M/config.json
cp $HERE/build.sh $HERE/test.sh $HERE/qemu.sh $M/
chmod +x $M/*.sh
echo setup done
