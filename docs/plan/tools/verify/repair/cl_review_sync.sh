#!/bin/bash
# cl_review_sync.sh   Append-only review todo for the DeepSeek stage's fixes: every key in dsv/done.keys that is not yet in review-dsv/todo.keys gets its "fix" results from dsv/dsv.jsonl
# turned into step-4 todo lines (cl_review_todo.sh) and appended to review-dsv/todo.tsv (line numbers stay stable: the batch runner refers to them). Read-only on mined-data/dsv.
HERE=$(cd "$(dirname "$0")" && pwd); CLD=/mnt/c/ParanoidBSD/mined-data/claude; DW=/mnt/c/ParanoidBSD/mined-data/dsv; R=$CLD/review-dsv
mkdir -p "$R"; touch "$R/todo.tsv" "$R/todo.keys"
exec 9> "$R/.sync.lock"; flock 9
sort -u "$DW/done.keys" > "$R/.done.now"
jq -r '"\(.key)\t\(tojson)"' "$DW/dsv.jsonl" 2> /dev/null | awk -F'\t' 'FILENAME == ARGV[1] { d[$1] = 1; next } FILENAME == ARGV[2] { t[$1] = 1; next } ($1 in d) && !($1 in t) { i = index($0, "\t"); print substr($0, i + 1) }' "$R/.done.now" "$R/todo.keys" - > "$R/.new.jsonl"
[ -s "$R/.new.jsonl" ] || exit 0
bash "$HERE/cl_review_todo.sh" "$R/.new.jsonl" "$R/.new.tsv" > /dev/null
cat "$R/.new.tsv" >> "$R/todo.tsv"; cut -f1 "$R/.new.tsv" >> "$R/todo.keys"
# keys whose fixes were all drops (no todo line) must not be re-scanned forever: mark every key of the new batch as seen
jq -r '.key' "$R/.new.jsonl" | sort -u >> "$R/todo.keys"
