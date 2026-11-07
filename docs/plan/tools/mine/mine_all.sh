#!/bin/bash
# mine_all.sh <tiers, e.g. "B C"> <budget-usd>   Mine whole corpora with GLM-5.3-Flash in batches of 400 chunks: resumable, spend cap, automatic back-off.
# Everything lives in /mnt/c/ParanoidBSD/mined-data (a persistent Windows folder OUTSIDE the repo; not /tmp):
#   tier<T>.jsonl raw results (about 4.5 KB per chunk) | chunks-tier<T>.txt the fixed, shuffled chunk list | progress-tier<T>.txt chunks already handed out
#   ledger.txt token ledger (spend) | failed.txt chunks that failed | errors.log API errors | miner.log this script's log | DONE-tier<T> when a tier is complete
# Tier B: core code (Linux kernel mm fs net block lib ipc security crypto init; HBSD sys/{kern,vm,net*,fs,ufs,geom,libkern,security,crypto,sys}; all of pbsd and kde)
# Tier C: everything else. Cost model: input $0.047/M, output $0.268/M tokens (an upper bound: cached input is cheaper).
# Env: MINE_P starting parallel calls (default 12; adapts between 4 and 32).
HERE=$(cd "$(dirname "$0")" && pwd); R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79; D=/mnt/c/ParanoidBSD/mined-data; L=$HOME/pbsd-ref/linux
TIERS=${1:-B}; BUDGET=${2:-60}; P=${MINE_P:-12}; mkdir -p "$D"
LED=$D/ledger.txt
# The ledger starts with what phase 1 and the pilot already cost, so the cap counts everything spent on mining.
if [ ! -f "$LED" ]; then
  for f in "$R/docs/plan/lib/code-mined/phase1.jsonl" "$R/docs/plan/lib/code-mined/pilot.jsonl"; do
    [ -f "$f" ] && jq -r '"\(.usage.prompt_tokens) \(.usage.completion_tokens)"' "$f" >> "$LED"
  done
fi
cost() { awk '{i+=$1; o+=$2} END{printf "%.4f", (i*0.047+o*0.268)/1000000}' "$LED"; }
over() { awk -v c="$(cost)" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; }
log() { echo "$(date '+%F %T') $*" | tee -a "$D/miner.log"; }

b_files() {
  for d in kernel mm fs net block lib ipc security crypto init; do find "$L/$d" -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null; done
  for d in kern vm net netinet netinet6 netpfil fs ufs geom libkern security crypto sys; do find "$R/hbsd/src/sys/$d" -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null; done
  find "$R/pbsd" "$R/kde" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null
}
all_files() {
  find "$L" "$R/hbsd" "$R/pbsd" "$R/kde" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null
}
chunker() {   # file start end, one chunk per line, cut at a blank line after 250 lines or at 300; files with spaces in the path or over 20000 lines are skipped
  while read -r f; do
    case "$f" in *" "*) continue ;; esac
    awk -v f="$f" 'BEGIN{s=1;n=0} {n++} (n-s>=250 && $0=="") || (n-s>=300){print f, s, n; s=n+1} END{if(n>=s&&n<=20000)print f, s, n}' "$f"
  done
}

for TIER in $TIERS; do
  OUT=$D/tier$TIER.jsonl; LIST=$D/chunks-tier$TIER.txt; PROG=$D/progress-tier$TIER.txt
  if [ ! -s "$LIST" ]; then
    log "building the chunk list for tier $TIER (reads every file once: a few minutes)"
    case $TIER in
      B) b_files | sort -u | chunker | shuf --random-source=<(yes 7) > "$LIST.part" ;;
      C) comm -23 <(all_files | sort -u) <(b_files | sort -u) | chunker | shuf --random-source=<(yes 7) > "$LIST.part" ;;
    esac
    mv "$LIST.part" "$LIST"
  fi
  TOTAL=$(wc -l < "$LIST"); i=$(cat "$PROG" 2>/dev/null || echo 0)
  log "tier $TIER: $TOTAL chunks, resume at $i, budget \$$BUDGET (spent so far \$$(cost)), parallel $P"
  while [ "$i" -lt "$TOTAL" ]; do
    if [ -f "$D/STOP" ]; then log "STOP file found: stopping in tier $TIER at chunk $i"; touch "$D/DONE-all"; exit 0; fi
    if over; then log "BUDGET \$$BUDGET reached (spent \$$(cost)): stopping in tier $TIER at chunk $i"; touch "$D/DONE-all"; exit 0; fi
    tail -n +$((i+1)) "$LIST" | head -n 400 > "$D/batch.cur"
    BEFORE=$(wc -l < "$D/failed.txt" 2>/dev/null || echo 0)
    xargs -P "$P" -L 1 bash -c "$HERE/mine_chunk.sh tier$TIER \$0 \$1 \$2 $OUT" < "$D/batch.cur" > /dev/null 2>> "$D/miner.err"
    FAILS=$(( $(wc -l < "$D/failed.txt" 2>/dev/null || echo 0) - BEFORE ))
    i=$((i+400)); [ "$i" -gt "$TOTAL" ] && i=$TOTAL; echo "$i" > "$PROG"
    if [ "$FAILS" -gt 40 ]; then P=$(( P/2 > 4 ? P/2 : 4 )); log "many failures ($FAILS in the batch): parallel down to $P, pausing 60 s"; sleep 60
    elif [ "$FAILS" -eq 0 ] && [ "$P" -lt 32 ]; then P=$((P+4)); fi
    log "tier $TIER chunk $i/$TOTAL results $(wc -l < "$OUT") spent \$$(cost) fails $FAILS parallel $P"
  done
  touch "$D/DONE-tier$TIER"; log "tier $TIER complete"
done
touch "$D/DONE-all"
log "MINE-ALL-DONE"
