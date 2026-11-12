#!/bin/bash
# repair_build.sh   Build the repair work list: every function tagged "rejected" (tags.jsonl), grouped by record, each with the concrete reasons it was rejected.
# Reasons come from: step 1 (s1_why), step 2 stub grammar (s2_why), Frama-C (its message), step 3 (the counterexample), step 4 (DeepSeek's reason; a tiebreak
# OVERTURN removes it). Output: mined-data/verify/repair/todo.tsv  (key, file, start, end, functions) where functions = "idx ¦ name ¦ prose ¦ contract ¦ problems"
# joined by " ¶ ". Records are ordered: formal contracts first, then the rest.
V=/mnt/c/ParanoidBSD/mined-data/verify; RP=${RP:-$V/repair}; mkdir -p "$RP"
jq -r 'select(.status=="rejected") | "\(.key)\t\(.i)"' "$V/tags.jsonl" > "$RP/rejected.tsv"
jq -r 'select(.s1=="fail" or .s2=="fail") | "\(.key)\t\(.i)\t" + ([(if .s1=="fail" then "mechanical: " + .s1_why else empty end), (if .s2=="fail" then "syntax: " + .s2_why else empty end)] | join(" | "))' "$V/step12.jsonl" > "$RP/p12.tsv"
jq -r 'select(.frama=="fail" or (.frama=="type" and (.msg|test("𝔹")))) | "\(.key)\t\(.i)\tFrama-C rejects the formal contract: \(.msg|.[0:120])"' "$V/frama12.jsonl" > "$RP/pfr.tsv"
jq -r 'select(.s3|test("^counterexample|^crash")) | "\(.key)\t\(.i)\trandom testing: \(if (.s3|test("crash")) then "the function traps on an input that meets the precondition" else "contract is FALSE for " + (.detail|sub("S3-COUNTEREXAMPLE ";"")) end)"' "$V/s3/s3.jsonl" > "$RP/ps3.tsv"
# step 4 reasons are per function NAME; the tiebreak overturns remove them
jq -r '.key as $k | .verdicts[] | select(.verdict=="disagree") | "\($k)\t\(.name)\tsecond reviewer: \(.reason)"' "$V/xcheck/xcheck.jsonl" > "$RP/px_byname.tsv"
cat "$V"/tiebreak/verdicts-*.tsv 2>/dev/null | awk -F'\t' 'tolower($2) == "overturn" { print $1 }' > "$RP/overturn.ids"
awk -F'\t' 'FILENAME == ARGV[1] { ov[$1] = 1; next } ($1 in ov) { print $2 "\t" $3 }' "$RP/overturn.ids" "$V/tiebreak/index.tsv" > "$RP/overturned.tsv"
# name -> index within its record, from rows.tsv (key idx name)
awk -F'\t' 'FILENAME == ARGV[1] { ov[$1 SUBSEP $2] = 1; next } FILENAME == ARGV[2] { if (!(($1 SUBSEP $2) in ov)) r[$1 SUBSEP $2] = $3; next }
            (($1 SUBSEP $6) in r) { print $1 "\t" $5 "\t" r[$1 SUBSEP $6] }' "$RP/overturned.tsv" "$RP/px_byname.tsv" "$V/rows.tsv" > "$RP/px.tsv"
# all problems per (key, idx)
cat "$RP/p12.tsv" "$RP/pfr.tsv" "$RP/ps3.tsv" "$RP/px.tsv" | awk -F'\t' '{ k = $1 SUBSEP $2; p[k] = (k in p) ? p[k] " | " $3 : $3 } END { for (k in p) { split(k, a, SUBSEP); print a[1] "\t" a[2] "\t" p[k] } }' > "$RP/problems.tsv"
# the work list: rows of rejected functions with their problems, grouped by record
awk -F'\t' 'FILENAME == ARGV[1] { rej[$1 SUBSEP $2] = 1; next } FILENAME == ARGV[2] { pr[$1 SUBSEP $2] = $3; next }
  (($1 SUBSEP $5) in rej) { k = $1; g = $5 " ¦ " $6 " ¦ " $9 " ¦ " $8 " ¦ " ((($1 SUBSEP $5) in pr) ? pr[$1 SUBSEP $5] : "rejected (see tags)")
    if (!(k in head)) { head[k] = $1 "\t" $2 "\t" $3 "\t" $4; ord[++n] = k; formal[k] = 0 }
    fn[k] = (k in fn) ? fn[k] " ¶ " g : g; if ($8 != "") formal[k] = 1 }
  END { for (i = 1; i <= n; i++) { k = ord[i]; print formal[k] "\t" head[k] "\t" fn[k] } }' "$RP/rejected.tsv" "$RP/problems.tsv" "$V/rows.tsv" |
  sort -s -t "$(printf '\t')" -k1,1nr | cut -f2- > "$RP/todo.tsv.part"
mv "$RP/todo.tsv.part" "$RP/todo.tsv"
echo "rejected functions: $(wc -l < "$RP/rejected.tsv"); with a recorded reason: $(wc -l < "$RP/problems.tsv"); records to repair: $(wc -l < "$RP/todo.tsv")"
