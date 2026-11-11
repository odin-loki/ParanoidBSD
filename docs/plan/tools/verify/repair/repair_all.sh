#!/bin/bash
# repair_all.sh <max-records> <budget-usd> [parallel]   Run repair_chunk.sh over repair/todo.tsv (from repair_build.sh) in batches of 1000: resumable (repaired keys are
# skipped), spend-capped (GLM-5.3-Flash on Morph: $0.047 / $0.268 per 1M tokens, an upper bound), STOP file honoured. Output: mined-data/verify/repair/repair.jsonl
HERE=$(cd "$(dirname "$0")" && pwd); RP=/mnt/c/ParanoidBSD/mined-data/verify/repair; OUT=$RP/repair.jsonl; LED=$RP/ledger.txt
MAXREC=${1:-100}; BUDGET=${2:-40}; P=${3:-48}
log() { echo "$(date '+%F %T') $*" | tee -a "$RP/repair.log"; }
cost() { awk '{i+=$1; o+=$2} END{printf "%.4f", (i*0.047+o*0.268)/1000000}' "$LED" 2>/dev/null || echo 0; }
rm -f "$RP/DONE"
[ -s "$OUT" ] && jq -r '.key' "$OUT" | sort -u > "$RP/done.keys" || : > "$RP/done.keys"
head -n "$MAXREC" "$RP/todo.tsv" | awk -F'\t' 'FILENAME == ARGV[1] { d[$1] = 1; next } !($1 in d)' "$RP/done.keys" - > "$RP/todo.now"
TOTAL=$(wc -l < "$RP/todo.now"); log "repair: $TOTAL records to do (cap $MAXREC), budget \$$BUDGET, spent so far \$$(cost), parallel $P"
i=0
while [ "$i" -lt "$TOTAL" ]; do
  [ -f "$RP/STOP" ] && { log "STOP file: stopping at $i"; break; }
  # out of credit (or a bad key): stop at once instead of racing through the list with failing calls; the next start resumes here
  if tail -n 50 "$RP/errors.log" 2>/dev/null | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key'; then
    log "ALERT credit or auth error from the API: stopping at $i (top up, then run start_repair.cmd again: it resumes)"; touch "$RP/STOP"; break; fi
  if awk -v c="$(cost)" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; then log "BUDGET \$$BUDGET reached (spent \$$(cost)): stopping at $i"; break; fi
  tail -n +$((i+1)) "$RP/todo.now" | head -n "${REPAIR_BATCH:-4000}" > "$RP/batch.cur"
  xargs -P "$P" -d '\n' -I{} bash "$HERE/repair_chunk.sh" "{}" "$OUT" < "$RP/batch.cur" 2>> "$RP/repair.err"
  i=$((i+${REPAIR_BATCH:-4000}))
  log "repair: $(wc -l < "$OUT") records repaired, spent \$$(cost), failed $(sort -u "$RP/failed.txt" 2>/dev/null | wc -l)"
done
log "repair batch finished: $(wc -l < "$OUT" 2>/dev/null || echo 0) records, spent \$$(cost)"
touch "$RP/DONE"
