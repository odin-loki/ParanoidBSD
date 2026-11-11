#!/bin/bash
# step4_all.sh <max-records> <budget-usd> [parallel]   Step 4 over the eligible subset: functions of kind state-machine or driver-glue that passed steps 1 and 2
# (s1 pass, s2 pass or skip). One DeepSeek V4.1 Flash 1017 call per record (step4_chunk.sh). Records are taken in a fixed order (formal contracts first, random within each
# group). Resumable (done keys are skipped), spend-capped, STOP file honoured.
# ONE continuous stream (11 Nov 2026): earlier versions ran batches and waited for each batch's slowest call (some take minutes, retries up to 15), so most workers idled
# at the end of every batch (rate swung between 400 and 1 record a minute). Now xargs runs the whole list; a background watchdog enforces the budget and the credit stop
# by touching $X/STOP (step4_chunk.sh checks it before every call). All data in /mnt/c/ParanoidBSD/mined-data/verify/
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data; V=$D/verify; X=$V/xcheck; mkdir -p "$X"
MAXREC=${1:-300}; BUDGET=${2:-20}; P=${3:-24}; OUT=$X/xcheck.jsonl; LED=$X/ledger.txt
log() { echo "$(date '+%F %T') $*" | tee -a "$X/step4.log"; }
# price: $0.12 / 1M input, $0.80 / 1M output (Morph list price on 17 Oct 2026; cached input is billed less, so this is an upper bound)
cost() { awk '{i+=$1; o+=$2} END{printf "%.4f", (i*0.12+o*0.80)/1000000}' "$LED" 2>/dev/null || echo 0; }
if [ ! -s "$V/s4todo.tsv" ]; then
  log "building the todo list (eligible records, formal contracts first): several minutes"
  touch "$X/BUILDING"; trap 'rm -f "$X/BUILDING"' EXIT
  jq -r 'select((.kind=="state-machine" or .kind=="driver-glue") and .s1=="pass" and (.s2=="pass" or .s2=="skip")) | "\(.key)\t\(.i)"' "$V/step12.jsonl" > "$V/s4elig.tsv"
  awk -F'\t' 'NR==FNR { el[$1 SUBSEP $2] = 1; next }
       (($1 SUBSEP $5) in el) { if ($1 != cur) { if (cur != "") print line; cur = $1; line = $1 "\t" $2 "\t" $3 "\t" $4 "\t"; first = 1 }
                                f = $6 " ¦ " $9 " ¦ " $8; line = line (first ? "" : " ¶ ") f; first = 0 }
       END { if (cur != "") print line }' "$V/s4elig.tsv" "$V/rows.tsv" | shuf --random-source=<(yes 11) |
    # records holding at least one FORMAL contract first (the formal-vs-prose defects are the main target), random order within each group
    awk -F'\t' '{ print (index($5, "/*@") ? 0 : 1) "\t" $0 }' | sort -s -t "$(printf '\t')" -k1,1n | cut -f2- > "$V/s4todo.tsv.part"
  mv "$V/s4todo.tsv.part" "$V/s4todo.tsv"; rm -f "$X/BUILDING"   # atomic: a killed build never leaves a truncated list behind
  log "eligible records: $(wc -l < "$V/s4todo.tsv")  (functions: $(wc -l < "$V/s4elig.tsv"))"
fi
[ -s "$OUT" ] && jq -r '.key' "$OUT" | sort -u > "$X/done.keys" || : > "$X/done.keys"
head -n "$MAXREC" "$V/s4todo.tsv" | awk -F'\t' 'FILENAME == ARGV[1] { d[$1] = 1; next } !($1 in d)' "$X/done.keys" - > "$X/todo.now"   # (NR==FNR is wrong when done.keys is empty)
TOTAL=$(wc -l < "$X/todo.now"); log "step 4: $TOTAL records to do (cap $MAXREC), budget \$$BUDGET, spent so far \$$(cost), parallel $P (continuous stream)"
rm -f "$X/DONE-batch"
# watchdog: stops the stream (STOP file) when the budget is reached or the API reports no credit / a bad key
( while [ ! -f "$X/DONE-batch" ] && [ ! -f "$X/STOP" ]; do
    sleep 20
    if awk -v c="$(cost)" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; then log "BUDGET \$$BUDGET reached (spent \$$(cost)): stopping"; touch "$X/STOP"; fi
    if tail -n 60 "$X/errors.log" 2>/dev/null | grep -i '"error"' | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key'; then log "ALERT credit or auth error from the API: stopping"; touch "$X/STOP"; fi
    log "step 4: $(wc -l < "$OUT") records done, spent \$$(cost), failed $(sort -u "$X/failed.txt" 2>/dev/null | wc -l)"
  done ) &
WD=$!
xargs -P "$P" -d '\n' -I{} bash "$HERE/step4_chunk.sh" "{}" "$OUT" < "$X/todo.now" 2>> "$X/step4.err"
kill "$WD" 2>/dev/null
log "step 4 finished: $(wc -l < "$OUT" 2>/dev/null || echo 0) records, spent \$$(cost)"
[ -f "$X/STOP" ] || touch "$X/DONE-batch"
