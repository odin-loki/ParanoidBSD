#!/bin/bash
# ingest_manual.sh <result.jsonl> [review-budget-usd=5] [parallel=100]
# Ingest the result of a manual packet: one JSON object per line {"key","i","name","action":"fix"|"drop","prose","contract","note"}  (key and i exactly as in the packet).
# It becomes the next repair round (round K = one more than the highest existing verify-repair<K>), then the same checks as every round run on just those functions:
# steps 1-3 + Frama-C (repair_verify.sh), the DeepSeek review of the fixed ones (step 4, spend capped by the budget), and state.sh merges the outcome into the status table.
# Afterwards: export_manual.sh again removes what now passes; tag_final.sh re-tags everything. Manual results are listed in repair<K>/manual-notes.jsonl (the notes).
RES=${1:?result.jsonl}; VB=${2:-5}; VPAR=${3:-100}
export NAME_WHOLE_FILE=1   # manual results: the function name may be found anywhere in the source file (the slice often cuts off the head); recorded as note "name outside slice"
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data
[ -s "$RES" ] || { echo "empty or missing $RES"; exit 1; }
jq -e -s 'all(.[]; has("key") and has("i") and (.action == "fix" or .action == "drop"))' "$RES" > /dev/null || { echo "bad result file: every line needs key, i, action fix|drop"; exit 1; }
K=2; while [ -d "$D/repair$K" ] || [ -d "$D/verify-repair$K" ]; do K=$((K+1)); done
RP=$D/repair$K; mkdir -p "$RP"; cp "$RES" "$RP/manual-notes.jsonl"
# record boundaries (file, start, end) from the original rows
jq -r '.key' "$RES" | sort -u > "$RP/keys.txt"
awk -F'\t' 'FILENAME == ARGV[1] { k[$1] = 1; next } ($1 in k) && !($1 in seen) { seen[$1] = 1; print $1 "\t" $2 "\t" $3 "\t" $4 }' "$RP/keys.txt" "$D/verify/rows.tsv" > "$RP/bounds.tsv"
jq -c -s --rawfile b "$RP/bounds.tsv" '($b | split("\n") | map(select(. != "") | split("\t") | {(.[0]): {file: .[1], start: (.[2]|tonumber), end: (.[3]|tonumber)}}) | add) as $B
   | group_by(.key) | map({key: .[0].key, file: $B[.[0].key].file, start: $B[.[0].key].start, end: $B[.[0].key].end,
       result: {functions: map({idx: .i, name: (.name // ""), action: .action, prose_contract: (.prose // ""), contract: (.contract // "")})}}) | .[]' "$RES" > "$RP/repair.jsonl"
echo "round $K: $(wc -l < "$RP/repair.jsonl") records from $(wc -l < "$RES") function results"
touch "$RP/retry-review.done" "$RP/todo.done" "$RP/repair.stage.done"
bash "$HERE/round_run.sh" "$K" 0 "$VB" 100 "$VPAR"
echo "state after:"; cat "$D/final/state-summary.json" | jq -c .by_status
