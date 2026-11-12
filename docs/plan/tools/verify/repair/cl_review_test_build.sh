#!/bin/bash
# cl_review_test_build.sh <out-dir> [n=50]   Reviewer test set: N known-good records (hand-fixed manual results, first function only) and N known-bad ones (same code, annotation of
# ANOTHER function: key suffix "#bad"). Output: <out-dir>/todo.tsv (2N lines, good/bad alternate), <out-dir>/truth.tsv (key<TAB>good|bad).
HERE=$(cd "$(dirname "$0")" && pwd); OD=${1:?out dir}; N=${2:-50}; mkdir -p "$OD"
cat /mnt/c/ParanoidBSD/mined-data/manual/results/*.jsonl | jq -c 'select(.action == "fix" and (.prose|length) > 40)' | awk -F'"key":' '!s[$2 (NR % 1)]++' > "$OD/good.jsonl"
jq -s -c 'group_by(.key) | map(.[0]) | .[]' "$OD/good.jsonl" | shuf --random-source=<(yes 7) | head -n "$N" > "$OD/pick.jsonl"
bash "$HERE/cl_review_todo.sh" "$OD/pick.jsonl" "$OD/good.tsv" > /dev/null
awk -F'\t' -v OFS='\t' '{ n = split($5, a, " ¶ "); split(a[1], f, " ¦ "); L[NR] = $1 OFS $2 OFS $3 OFS $4; NM[NR] = f[1]; PR[NR] = f[2]; CT[NR] = f[3] }
  END { for (i = 1; i <= NR; i++) { j = (i % NR) + 1
          print L[i] OFS NM[i] " ¦ " PR[i] " ¦ " CT[i]
          split(L[i], h, OFS); print h[1] "#bad" OFS h[2] OFS h[3] OFS h[4] OFS NM[i] " ¦ " PR[j] " ¦ " CT[j] } }' "$OD/good.tsv" > "$OD/todo.tsv"
awk -F'\t' '{ print $1 "\t" ($1 ~ /#bad$/ ? "bad" : "good") }' "$OD/todo.tsv" > "$OD/truth.tsv"
echo "$(wc -l < "$OD/todo.tsv") lines"
