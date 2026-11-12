#!/bin/bash
# cl_cands.sh <mode> [jobdir]   Candidate records, "key<TAB>line" on stdout in priority order, AFTER removing everything already taken care of:
#   keys in the DeepSeek stage's done.keys (re-read at every call), keys of ANY Claude job's claimed.keys (submitted, not failed) and gave-up keys.
#   mode opus      group 0 (still-rejected) records, then records the DeepSeek stage gave up on (dsv/failed.txt = work.jsonl line numbers)
#   mode tail      the END of the DeepSeek order, backwards (all groups): the records the DeepSeek budget will not reach
#   mode groupN    records of group N in DeepSeek order (for tests)
MODE=$1; CLD=/mnt/c/ParanoidBSD/mined-data/claude; DW=/mnt/c/ParanoidBSD/mined-data/dsv
{ cat "$DW/done.keys" 2>/dev/null; cat "$CLD"/jobs/*/claimed.keys 2>/dev/null; cat "$CLD"/jobs/*/gaveup.keys 2>/dev/null; } | sort -u > "$CLD/.taken.$$"
case "$MODE" in
  opus)
    { awk -F'\t' '$3 == 0 { print $1 "\t" $2 }' "$CLD/groups.tsv"
      sort -u "$DW/failed.txt" 2>/dev/null | awk -F'\t' 'FILENAME == ARGV[1] { k[$2] = $1; next } ($1 in k) { print k[$1] "\t" $1 }' "$CLD/groups.tsv" - ; } ;;
  tail)  # the DeepSeek stage works from the front of the order until its budget is spent (~position 17000 at 55 USD); to avoid a second payment for the same record we stay behind
         # position FLOOR while it is running (default 16500); once its STOP/DONE file exists the floor is gone and only its done.keys matter
         FLOOR=${TAILFLOOR:-16500}; { [ -f "$DW/STOP" ] || [ -f "$DW/DONE" ]; } && FLOOR=0
         awk -F'\t' -v fl="$FLOOR" 'FILENAME == ARGV[1] { n[$1] = $2; next } (FNR > fl) && ($1 in n) { print $1 "\t" n[$1] }' "$CLD/groups.tsv" "$CLD/order.tsv" | tac ;;
  group*) g=${MODE#group}; awk -F'\t' -v g="$g" '$3 == g { print $1 "\t" $2 }' "$CLD/groups.tsv" ;;
esac | awk -F'\t' 'FILENAME == ARGV[1] { t[$1] = 1; next } !($1 in t) && !($1 in seen) { seen[$1] = 1; print }' "$CLD/.taken.$$" -
rm -f "$CLD/.taken.$$"
