#!/bin/bash
# dsv_all.sh <budget-usd> [parallel]   One continuous xargs stream over $DW/work.jsonl (dsv_build.sh), resumable (record keys in done.keys are skipped), spend-capped
# (upper bound at $0.12/$0.80 per 1M from ledger.txt). A watchdog touches $DW/STOP on the budget or on a credit/auth error; dsv_chunk.sh checks STOP before each call.
# Env: MAXN = max records this run, DSV_EFFORT (medium), DSV_MAXTOK (24000).
HERE=$(cd "$(dirname "$0")" && pwd); DW=${DW:-/mnt/c/ParanoidBSD/mined-data/dsv}; OUT=$DW/dsv.jsonl; LED=$DW/ledger.txt
BUDGET=${1:-40}; P=${2:-64}; MAXN=${MAXN:-999999}
log() { echo "$(date '+%F %T') $*" | tee -a "$DW/repair.log"; }
cost() { awk '{i+=$1; o+=$2} END{printf "%.4f", (i*0.12+o*0.80)/1000000}' "$LED" 2>/dev/null || echo 0; }
rm -f "$DW/DONE"; touch "$DW/done.keys"
awk -F'\t' 'FILENAME == ARGV[1] { d[$1] = 1; next } !($1 in d) { print $2 }' "$DW/done.keys" "$DW/order.tsv" | head -n "$MAXN" > "$DW/todo.now"
log "dsv repair: $(wc -l < "$DW/todo.now") records to do, budget \$$BUDGET, spent \$$(cost), parallel $P"
( while [ ! -f "$DW/DONE" ] && [ ! -f "$DW/STOP" ]; do
    sleep 20
    if awk -v c="$(cost)" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; then log "BUDGET \$$BUDGET reached (spent \$$(cost)): stopping"; touch "$DW/STOP"; fi
    if tail -n 60 "$DW/errors.log" 2>/dev/null | grep -i 'attempt' | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key'; then log "ALERT credit or auth error from the API: stopping (top up, delete STOP, run again: it resumes)"; touch "$DW/STOP"; fi
    log "dsv: $(sort -u "$DW/done.keys" | wc -l) records done, $(wc -l < "$OUT" 2>/dev/null || echo 0) function results, spent \$$(cost), failed $(sort -u "$DW/failed.txt" 2>/dev/null | wc -l)"
  done ) &
WD=$!
xargs -P "$P" -I{} bash "$HERE/dsv_chunk.sh" "{}" "$OUT" < "$DW/todo.now" 2>> "$DW/repair.err"
kill "$WD" 2>/dev/null
log "dsv repair finished: $(sort -u "$DW/done.keys" | wc -l) records, spent \$$(cost)"
[ -f "$DW/STOP" ] || touch "$DW/DONE"
