#!/bin/bash
# repair_all.sh <max-records> <budget-usd> [parallel]   Run repair_chunk.sh over repair/todo.tsv (from repair_build.sh) as ONE continuous stream: resumable (repaired keys
# are skipped), spend-capped (GLM-5.3-Flash on Morph: $0.047 / $0.268 per 1M tokens, an upper bound). A background watchdog enforces the budget and the credit stop by
# touching $RP/STOP, which repair_chunk.sh checks before every call (no batch barrier: batches left the workers idle while their slowest call finished).
# Output: mined-data/verify/repair/repair.jsonl
HERE=$(cd "$(dirname "$0")" && pwd); RP=/mnt/c/ParanoidBSD/mined-data/verify/repair; OUT=$RP/repair.jsonl; LED=$RP/ledger.txt
MAXREC=${1:-100}; BUDGET=${2:-40}; P=${3:-48}
log() { echo "$(date '+%F %T') $*" | tee -a "$RP/repair.log"; }
cost() { awk '{i+=$1; o+=$2} END{printf "%.4f", (i*0.047+o*0.268)/1000000}' "$LED" 2>/dev/null || echo 0; }
rm -f "$RP/DONE"
[ -s "$OUT" ] && jq -r '.key' "$OUT" | sort -u > "$RP/done.keys" || : > "$RP/done.keys"
head -n "$MAXREC" "$RP/todo.tsv" | awk -F'\t' 'FILENAME == ARGV[1] { d[$1] = 1; next } !($1 in d)' "$RP/done.keys" - > "$RP/todo.now"
TOTAL=$(wc -l < "$RP/todo.now"); log "repair: $TOTAL records to do (cap $MAXREC), budget \$$BUDGET, spent so far \$$(cost), parallel $P (continuous stream)"
( while [ ! -f "$RP/DONE" ] && [ ! -f "$RP/STOP" ]; do
    sleep 20
    if awk -v c="$(cost)" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; then log "BUDGET \$$BUDGET reached (spent \$$(cost)): stopping"; touch "$RP/STOP"; fi
    if tail -n 60 "$RP/errors.log" 2>/dev/null | grep -i '"error"' | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key'; then log "ALERT credit or auth error from the API: stopping (top up, then run start_repair.cmd again: it resumes)"; touch "$RP/STOP"; fi
    log "repair: $(wc -l < "$OUT") records repaired, spent \$$(cost), failed $(sort -u "$RP/failed.txt" 2>/dev/null | wc -l)"
  done ) &
WD=$!
xargs -P "$P" -d '\n' -I{} bash "$HERE/repair_chunk.sh" "{}" "$OUT" < "$RP/todo.now" 2>> "$RP/repair.err"
kill "$WD" 2>/dev/null
log "repair finished: $(wc -l < "$OUT" 2>/dev/null || echo 0) records, spent \$$(cost)"
[ -f "$RP/STOP" ] || touch "$RP/DONE"
