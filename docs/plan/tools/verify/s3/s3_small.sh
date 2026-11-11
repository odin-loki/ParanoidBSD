#!/bin/bash
# s3_small.sh   Re-run every step 3 counterexample with small non-negative inputs only (0..2000; negative values would wrap to huge ones for unsigned parameters). A contract that then holds is wrong only at the integer limits (it ignores
# wraparound or the type range: a + 1 at INT_MAX, a 64-bit shift); one that still fails is wrong for ordinary values (a wrong constant, a precedence bug, wrong logic).
# Output: mined-data/verify/s3/small.tsv (id, status) and s3.jsonl rewritten with s3 = counterexample-logic | counterexample-limits.
HERE=$(cd "$(dirname "$0")" && pwd); V=/mnt/c/ParanoidBSD/mined-data/verify; O=$V/s3; S=$HOME/s3-scratch; T=$S/small; rm -rf "$T"; mkdir -p "$T"
awk -F'\t' '$2 == "counterexample" { print $1 }' "$S/results.tsv" > "$T/ids.txt"
while read -r id; do
  cp "$S/t/f_$id.c" "$T/"
  sed 's/switch (r & 3).*$/(void)r; return (long long)(s3_rand() % 2001); }/' "$S/t/t_$id.c" > "$T/t_$id.c"
done < "$T/ids.txt"
xargs -P 32 -I{} bash "$HERE/s3_one.sh" "$T" {} < "$T/ids.txt" > "$O/small.tsv"
echo "small-input rerun of $(wc -l < "$T/ids.txt") counterexamples:"; cut -f2 "$O/small.tsv" | sort | uniq -c
# rewrite s3.jsonl: counterexample -> counterexample-logic (still fails with small inputs) or counterexample-limits (passes with small inputs)
awk -F'\t' 'FILENAME == ARGV[1] { st[$1] = $2; next } FILENAME == ARGV[2] { meta[$2 SUBSEP $3] = $1; next }
  { line = $0
    if (line ~ /"s3":"counterexample"/) {
      match(line, /"key":"[^"]*"/); k = substr(line, RSTART + 7, RLENGTH - 8); match(line, /"i":[0-9]+/); i = substr(line, RSTART + 4, RLENGTH - 4)
      id = meta[k SUBSEP i]; v = (id in st && st[id] == "pass") ? "counterexample-limits" : "counterexample-logic"
      sub(/"s3":"counterexample"/, "\"s3\":\"" v "\"", line) }
    print line }' "$O/small.tsv" "$S/meta.tsv" "$O/s3.jsonl" > "$O/s3.jsonl.new" && mv "$O/s3.jsonl.new" "$O/s3.jsonl"
jq -s '{candidates: length, s3: (group_by(.s3)|map({(.[0].s3): length})|add)}' "$O/s3.jsonl" | tee "$O/summary.json"
