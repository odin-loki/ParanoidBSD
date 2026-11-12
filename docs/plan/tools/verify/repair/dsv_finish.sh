#!/bin/bash
# dsv_finish.sh <review-budget-usd> [parallel=100]   After the dsv repair stage: wait until no other ingest/state/tag_final/export process runs and the shared scripts are committed,
# then ingest mined-data/dsv/dsv.jsonl as ONE manual-result file (ingest_manual.sh: re-verify steps 1-3 + Frama-C, review by step 4, state.sh), then export_manual.sh and tag_final.sh.
# The reviewer (step4, morph-dsv41flash-1017 with the unchanged reviewer prompt) is the SAME model as the fixer here. Dedupes (key,i): the last line wins. Never touches manual/results/.
VB=${1:?review budget}; VPAR=${2:-100}
HERE=$(cd "$(dirname "$0")" && pwd); DW=/mnt/c/ParanoidBSD/mined-data/dsv; REPO=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79
log() { echo "$(date '+%F %T') [finish] $*" | tee -a "$DW/finish.log"; }
until [ -f "$DW/DONE" ] || [ -f "$DW/STOP" ] || [ -f "$DW/ingest.started" ]; do sleep 30; done   # the repair stage ends by finishing the list (DONE) or by the budget (STOP)
[ -f "$DW/ingest.started" ] || while pgrep -f 'dsv_chunk.sh|dsv_all.sh' > /dev/null; do sleep 10; done   # in-flight calls finish first
if [ ! -f "$DW/ingest.started" ] && tail -n 5 "$DW/repair.log" | grep -q "ALERT credit"; then log "credit/auth alert in the repair stage: not ingesting; top up, restart start_dsv.cmd"; exit 2; fi
busy() { pgrep -f 'ingest_manual.sh|/state.sh|tag_final.sh|export_manual.sh|round_run.sh' | grep -vx "$$" > /dev/null; }
[ -f "$DW/ingest.started" ] || {
  while busy; do log "waiting: another ingest/state/tag_final/export/round process is running"; sleep 120; done
  while [ -n "$(git -C "$REPO" status --porcelain -- docs/plan/tools/verify/verify12.awk docs/plan/tools/verify/repair/ingest_manual.sh docs/plan/tools/verify/repair/state.sh)" ]; do log "waiting: verify12.awk/ingest_manual.sh/state.sh have uncommitted edits (another agent)"; sleep 120; done
  jq -c -s 'group_by([.key, .i]) | map(.[-1]) | .[]' "$DW/dsv.jsonl" > "$DW/dsv.dedup.jsonl"
  log "ingest: $(wc -l < "$DW/dsv.dedup.jsonl") function results ($(jq -s '[.[]|select(.action=="drop")]|length' "$DW/dsv.dedup.jsonl") drops)"
  touch "$DW/ingest.started"
  bash "$HERE/ingest_manual.sh" "$DW/dsv.dedup.jsonl" "$VB" "$VPAR" 2>&1 | tee -a "$DW/ingest.log"
}
while busy; do log "waiting: another process is running"; sleep 60; done
bash "$HERE/state.sh" > "$DW/state-after.json" && bash "$HERE/export_manual.sh" 40 | tee -a "$DW/finish.log" && bash "$HERE/tag_final.sh" | tee -a "$DW/finish.log"
touch "$DW/FINISHED"; log "finished"
