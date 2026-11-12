#!/bin/bash
# cl_score.sh <result.jsonl> <scratch-dir> [sfx]   Score manual-format results with the SAME checks the ingest uses (verify12 steps 1-2, Frama-C, step 3; NAME_WHOLE_FILE=1)
# on just those functions, in a scratch directory. It does NOT touch the shared status table, the verify-repair<K> rounds or the manual results.
# Output: <scratch-dir>/verify/repair-tags.jsonl + summary.json ({"key","i","name","action","after":{...},"status":"fixed|still-rejected|dropped"}), <scratch-dir>/score.txt
RES=${1:?result.jsonl}; SD=${2:?scratch dir}; SFX=${3:-clscore}
export NAME_WHOLE_FILE=1
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data
mkdir -p "$SD"; W=$SD/verify
jq -r '.key' "$RES" | sort -u > "$SD/keys.txt"
awk -F'\t' 'FILENAME == ARGV[1] { k[$1] = 1; next } ($1 in k) && !($1 in seen) { seen[$1] = 1; print $1 "\t" $2 "\t" $3 "\t" $4 }' "$SD/keys.txt" "$D/verify/rows.tsv" > "$SD/bounds.tsv"
jq -c -s --rawfile b "$SD/bounds.tsv" '($b | split("\n") | map(select(. != "") | split("\t") | {(.[0]): {file: .[1], start: (.[2]|tonumber), end: (.[3]|tonumber)}}) | add) as $B
   | group_by(.key) | map({key: .[0].key, file: $B[.[0].key].file, start: $B[.[0].key].start, end: $B[.[0].key].end,
       result: {functions: map({idx: .i, name: (.name // ""), action: .action, prose_contract: (.prose // ""), contract: (.contract // "")})}}) | .[]' "$RES" > "$SD/repair.jsonl"
W=$W RJ=$SD/repair.jsonl SFX=$SFX bash "$HERE/repair_verify.sh" > "$SD/verify.out" 2>&1
{ echo "functions $(wc -l < "$W/repair-tags.jsonl")"; jq -r '.status' "$W/repair-tags.jsonl" | sort | uniq -c; } | tee "$SD/score.txt"
