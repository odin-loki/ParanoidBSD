#!/bin/bash
# cl_apply_reviews.sh <K> [replace|union]   Put the Claude (Haiku) reviews of the DeepSeek fixes (mined-data/claude/jobs/review-dsv/res.jsonl, step-4 xcheck format) into the xcheck file of repair round K
# (mined-data/verify-repair<K>/xcheck/xcheck.jsonl), for the keys of that round only, then state.sh is NOT run here (run it yourself after the ingest, serially).
#   replace (default): for a key that has a Claude review, the existing DeepSeek review line of that key is dropped (the fixer's own family does not review itself).
#   union: both reviews are kept; state.sh then lets a DISAGREE from either reviewer win.
# Typical use after dsv_finish.sh / ingest_manual.sh created round K (ingest with a small review budget, e.g. 0.5): bash cl_apply_reviews.sh K && bash state.sh
K=${1:?round K}; MODE=${2:-replace}; D=/mnt/c/ParanoidBSD/mined-data
if [ "$K" = 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$K; fi
CR=$D/claude/jobs/review-dsv/res.jsonl; X=$W/xcheck/xcheck.jsonl
[ -s "$W/rows.tsv" ] || { echo "round $K has no rows.tsv"; exit 1; }
[ -s "$CR" ] || { echo "no Claude reviews yet: $CR"; exit 1; }
mkdir -p "$W/xcheck"; touch "$X"
cut -f1 "$W/rows.tsv" | sort -u > "$W/xcheck/.keys.k"
jq -c 'select(.key != null)' "$CR" | jq -r '"\(.key)\t\(tojson)"' | awk -F'\t' 'FILENAME == ARGV[1] { k[$1] = 1; next } ($1 in k) { i = index($0, "\t"); print substr($0, i + 1) }' "$W/xcheck/.keys.k" - > "$W/xcheck/.claude.part"
cut -f1 "$W/xcheck/.keys.k" > /dev/null
jq -r '.key' "$W/xcheck/.claude.part" | sort -u > "$W/xcheck/.claude.keys"
cp "$X" "$W/xcheck/xcheck.jsonl.before-claude"
if [ "$MODE" = replace ]; then
  jq -r '"\(.key)\t\(tojson)"' "$X" | awk -F'\t' 'FILENAME == ARGV[1] { c[$1] = 1; next } !($1 in c) { i = index($0, "\t"); print substr($0, i + 1) }' "$W/xcheck/.claude.keys" - > "$W/xcheck/.keep"
else cp "$X" "$W/xcheck/.keep"; fi
cat "$W/xcheck/.keep" "$W/xcheck/.claude.part" > "$X"
echo "round $K: $(wc -l < "$W/xcheck/.claude.part") Claude review lines for $(wc -l < "$W/xcheck/.claude.keys") keys applied ($MODE); xcheck.jsonl now $(wc -l < "$X") lines (backup xcheck.jsonl.before-claude). Next: bash state.sh"
