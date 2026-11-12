#!/bin/bash
# build_todo.sh <round>   Repair work list of round N from final/state.tsv (run state.sh first): every function that is still bad:
#   tier 0: fixed-disagree (reviewer's reason) and still-rejected (the failing check) from earlier rounds
#   tier 1: rejected-unrepaired, kinds other than driver-glue   (never repaired: round 1 ran before step 4 finished, or its repair call failed)
#   tier 2: rejected-unrepaired, driver-glue
# The annotation shown to the repair model is the CURRENT one (latest round that rewrote it, else the original).
# Output: mined-data/repair<N>/todo.tsv in the repair_chunk.sh format (key, file, start, end, "idx ¦ name ¦ prose ¦ contract ¦ problems" joined by " ¶ "),
# records ordered by their best tier, formal-contract records first inside a tier.
N=${1:?round}; D=/mnt/c/ParanoidBSD/mined-data; F=$D/final; RP=$D/repair$N; mkdir -p "$RP"
ROWS="$D/verify/rows.tsv"; for k in $(seq 1 $((N-1))); do if [ $k -eq 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$k; fi; [ -s "$W/rows.tsv" ] && ROWS="$ROWS $W/rows.tsv"; done
awk -F'\t' '$5 == "fixed-disagree" || $5 == "still-rejected" || $5 == "rejected-unrepaired" { t = ($5 == "rejected-unrepaired") ? (($4 == "driver-glue") ? 2 : 1) : 0; print $1 "\t" $2 "\t" t "\t" $7 }' "$F/state.tsv" > "$RP/items.tsv"
awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { it[$1 SUBSEP $2] = $3 "\t" $4; next }
     { q = $1 SUBSEP $5; if (q in it) { row[q] = $0 } }
     END { for (q in row) { split(row[q], r, "\t"); split(it[q], t, "\t"); gsub(/[\t\n]/, " ", t[2]); gsub(/ ¦ | ¶ /, " ", t[2]); gsub(/ ¦ | ¶ /, " ", r[8]); gsub(/ ¦ | ¶ /, " ", r[9])
             print r[1], r[2], r[3], r[4], r[5], r[6], t[1], (r[8] != "" ? 1 : 0), r[9], r[8], t[2] } }' "$RP/items.tsv" $ROWS |
  sort -s -t "$(printf '\t')" -k1,1 -k5,5n > "$RP/funcs.tsv"
awk -F'\t' -v OFS='\t' 'function flush() { if (cur != "") print tier, 1 - formal, head, fn; }
     { if ($1 != cur) { flush(); cur = $1; head = $1 "\t" $2 "\t" $3 "\t" $4; fn = ""; tier = 9; formal = 0 }
       fn = fn (fn == "" ? "" : " ¶ ") $5 " ¦ " $6 " ¦ " $9 " ¦ " $10 " ¦ " ($11 == "" ? "rejected (see tags)" : $11)
       if ($7 < tier) tier = $7; if ($8 == 1) formal = 1 }
     END { flush() }' "$RP/funcs.tsv" | sort -s -t "$(printf '\t')" -k1,1n -k2,2n | cut -f3- > "$RP/todo.tsv.part"
mv "$RP/todo.tsv.part" "$RP/todo.tsv"
echo "round $N: items $(wc -l < "$RP/items.tsv"), functions with a current annotation $(wc -l < "$RP/funcs.tsv"), records $(wc -l < "$RP/todo.tsv")"
awk -F'\t' '{ c[$7]++ } END { for (t in c) print "  tier " t ": " c[t] " functions" }' "$RP/funcs.tsv" | sort
