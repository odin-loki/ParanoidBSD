#!/bin/bash
# cl_finish.sh <review-budget-usd> [parallel=100] [job...]   Ingest the Claude repair results (default jobs: sonnet-tail opus-fail) as ONE manual-result file, after everything else is quiet.
# NOT started automatically. Waits until no ingest/state/tag_final/export/round process runs and the shared scripts have no uncommitted edits (same rule as dsv_finish.sh), then
# ingest_manual.sh (re-verify steps 1-3 + Frama-C, DeepSeek step-4 review of the Claude fixes = an independent family, state.sh), export_manual.sh, tag_final.sh.
# Run it only AFTER dsv_finish.sh has FINISHED (mined-data/dsv/FINISHED), because the ingest is one serial step.
VB=${1:?review budget}; VPAR=${2:-100}; shift 2 2>/dev/null; JOBS=${*:-sonnet-tail opus-fail}
HERE=$(cd "$(dirname "$0")" && pwd); CLD=/mnt/c/ParanoidBSD/mined-data/claude; DW=/mnt/c/ParanoidBSD/mined-data/dsv; REPO=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79
log() { echo "$(date '+%F %T') [cl_finish] $*" | tee -a "$CLD/finish.log"; }
busy() { pgrep -f 'ingest_manual.sh|/state.sh|tag_final.sh|export_manual.sh|round_run.sh|dsv_finish.sh' | grep -vx "$$" > /dev/null; }
until [ -f "$DW/FINISHED" ]; do log "waiting for the DeepSeek stage to finish (mined-data/dsv/FINISHED)"; sleep 120; done
while busy; do log "waiting: another ingest/state/tag_final/export/round process is running"; sleep 120; done
while [ -n "$(git -C "$REPO" status --porcelain -- docs/plan/tools/verify/verify12.awk docs/plan/tools/verify/repair/ingest_manual.sh docs/plan/tools/verify/repair/state.sh)" ]; do log "waiting: verify12.awk/ingest_manual.sh/state.sh have uncommitted edits"; sleep 120; done
for j in $JOBS; do cat "$CLD/jobs/$j/res.jsonl" 2>/dev/null; done | jq -c -s 'group_by([.key, .i]) | map(.[-1]) | .[]' > "$CLD/claude.dedup.jsonl"
log "ingest: $(wc -l < "$CLD/claude.dedup.jsonl") function results ($(jq -s '[.[]|select(.action=="drop")]|length' "$CLD/claude.dedup.jsonl") drops) from: $JOBS"
bash "$HERE/ingest_manual.sh" "$CLD/claude.dedup.jsonl" "$VB" "$VPAR" 2>&1 | tee -a "$CLD/ingest.log"
while busy; do sleep 60; done
bash "$HERE/state.sh" > "$CLD/state-after.json" && bash "$HERE/export_manual.sh" 40 | tee -a "$CLD/finish.log" && bash "$HERE/tag_final.sh" | tee -a "$CLD/finish.log"
touch "$CLD/FINISHED"; log "finished"
