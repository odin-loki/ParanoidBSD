#!/bin/bash
# s4_repaired_build.sh   Builds the step 4 todo list for the REPAIRED annotations: records that hold at least one function tagged "fixed" by repair_verify.sh, containing only those functions.
# Output: verify-repair/s4todo.tsv (same format as verify/s4todo.tsv). Run step 4 on it with: S4_V=<verify-repair> S4_X=<verify-repair>/xcheck bash step4_all.sh <cap> <budget> <parallel>
W=${W:-/mnt/c/ParanoidBSD/mined-data/verify-repair}
jq -r 'select(.status=="fixed") | "\(.key)\t\(.i)"' "$W/repair-tags.jsonl" > "$W/s4elig.tsv"
awk -F'\t' 'FILENAME == ARGV[1] { el[$1 SUBSEP $2] = 1; next }
     (($1 SUBSEP $5) in el) { if ($1 != cur) { if (cur != "") print line; cur = $1; line = $1 "\t" $2 "\t" $3 "\t" $4 "\t"; first = 1 }
                              f = $6 " ¦ " $9 " ¦ " $8; line = line (first ? "" : " ¶ ") f; first = 0 }
     END { if (cur != "") print line }' "$W/s4elig.tsv" "$W/rows.tsv" | shuf --random-source=<(yes 12) > "$W/s4todo.tsv.part"
mv "$W/s4todo.tsv.part" "$W/s4todo.tsv"; echo "records: $(wc -l < "$W/s4todo.tsv")  functions: $(wc -l < "$W/s4elig.tsv")"
