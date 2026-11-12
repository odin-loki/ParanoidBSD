#!/bin/bash
# cl_review_cands.sh   Candidates of the review job: sync the todo with the DeepSeek stage's finished records, then "key<TAB>todo line number" for every todo line.
HERE=$(cd "$(dirname "$0")" && pwd); T=/mnt/c/ParanoidBSD/mined-data/claude/review-dsv/todo.tsv
bash "$HERE/cl_review_sync.sh"
awk -F'\t' '{ print $1 "\t" NR }' "$T"
