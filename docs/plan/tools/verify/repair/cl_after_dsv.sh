#!/bin/bash
# cl_after_dsv.sh [claude-review-budget-usd=8] [redoD-review-budget-usd=2.5] [parallel=100]
# Unattended tail of the Claude stage, serial, each step only when nothing else runs (same busy rule as dsv_finish.sh / cl_finish.sh). No call to api.anthropic.com: only the DeepSeek reviewer (MORPH key) is used.
#   0. wait for mined-data/dsv/FINISHED (dsv_finish.sh: dsv ingest = round K, state, export, tag)
#   1. Haiku verdicts on the DeepSeek fixes (claude/jobs/review-dsv/res.jsonl) into round K's xcheck (cl_apply_reviews.sh K replace), then state.sh
#   2. redo-D (19 hand-verified functions, manual/results/redo-D.jsonl; its first ingest, round 15, was killed in the re-verify step and moved to repair15.aborted-redoD) as round K+1, unless a round already holds exactly that file
#   3. cl_finish.sh: the Claude results sonnet-tail + opus-fail, deduplicated against the status table (a result for a function that is already fixed-agree is dropped; the DeepSeek result wins where it was accepted),
#      ingested as the next round, then state.sh, export_manual.sh, tag_final.sh
# Log: mined-data/claude/after_dsv.log ; result counts: mined-data/claude/after_dsv.summary ; markers after_dsv.step1/2/3.done make it resumable.
VB=${1:-8}; VB2=${2:-2.5}; VPAR=${3:-100}
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data; DW=$D/dsv; CLD=$D/claude
log() { echo "$(date '+%F %T') [after_dsv] $*" | tee -a "$CLD/after_dsv.log"; }
busy() { pgrep -f 'ingest_manual.sh|/state.sh|tag_final.sh|export_manual.sh|round_run.sh|dsv_finish.sh|cl_finish.sh' | grep -vx "$$" > /dev/null; }
wait_idle() { while busy; do log "waiting: another ingest/state/tag/export/round process is running"; sleep 60; done; }
until [ -f "$DW/FINISHED" ]; do sleep 60; done
K=$(sed -n 's/^round \([0-9]*\): .*/\1/p' "$DW/ingest.log" | head -1); [ -n "$K" ] || { log "cannot find the dsv round number in $DW/ingest.log"; exit 1; }
log "dsv finished; its round is $K"
if [ ! -f "$CLD/after_dsv.step1.done" ]; then
  wait_idle
  bash "$HERE/cl_apply_reviews.sh" "$K" replace 2>&1 | tee -a "$CLD/after_dsv.log"
  bash "$HERE/state.sh" > "$CLD/state-after-haiku.json" 2>&1; jq -c .by_status "$D/final/state-summary.json" | tee -a "$CLD/after_dsv.log"
  touch "$CLD/after_dsv.step1.done"
fi
if [ ! -f "$CLD/after_dsv.step2.done" ]; then
  wait_idle
  RD=$D/manual/results/redo-D.jsonl; have=
  for f in "$D"/repair*/manual-notes.jsonl; do case "$f" in *aborted*) continue;; esac; cmp -s "$f" "$RD" && have=$f; done
  if [ -n "$have" ]; then log "redo-D is already in $have: skipped"; else
    log "redo-D: ingest (review budget \$$VB2)"; bash "$HERE/ingest_manual.sh" "$RD" "$VB2" "$VPAR" 2>&1 | tee -a "$D/manual/ingest-redoD.log" | tee -a "$CLD/after_dsv.log"; fi
  touch "$CLD/after_dsv.step2.done"
fi
if [ ! -f "$CLD/after_dsv.step3.done" ]; then
  wait_idle
  log "Claude results: cl_finish.sh $VB $VPAR"
  bash "$HERE/cl_finish.sh" "$VB" "$VPAR" 2>&1 | tail -n 40 | tee -a "$CLD/after_dsv.log"
  touch "$CLD/after_dsv.step3.done"
fi
{ echo "claude results: dedup $(wc -l < "$CLD/claude.dedup.jsonl") function results, skipped as already fixed-agree $(wc -l < "$CLD/claude.skipped.tsv"), ingested $(wc -l < "$CLD/claude.filtered.jsonl")"
  echo "state: $(jq -c .by_status "$D/final/state-summary.json")"
  for r in $(seq "$K" 40); do [ -d "$D/repair$r" ] || break; echo "round $r: $(jq -c --arg r " r$r" '.by_status_round | to_entries | map(select(.key | endswith($r))) | from_entries' "$D/final/state-summary.json")"; done
} > "$CLD/after_dsv.summary"; cat "$CLD/after_dsv.summary" | tee -a "$CLD/after_dsv.log"
log "all done"
