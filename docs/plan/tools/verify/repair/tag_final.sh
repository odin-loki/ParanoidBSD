#!/bin/bash
# tag_final.sh   Re-tag ALL mined functions after the repair rounds, from final/state.tsv (run state.sh first). Every count comes from this script.
# Output: mined-data/final/final-tags.jsonl, one line per function:
#   {"key","i","name","kind","status":"verified|unchecked|rejected|dropped","origin":"kept as-is|fixed in round N|dropped in round N|...","reason"}
#   status verified    = original tag verified (kept as-is), or repaired and every check passes and the DeepSeek review agrees
#   status unchecked   = original tag unchecked (kept as-is), or repaired but the review has no verdict (failed call)
#   status rejected    = still rejected after the last round it went through (reason given), or never repaired
#   status dropped     = the repair model said it is not a real function (listed, not hidden)
# Repo copy: docs/plan/lib/code-mined/final-tags/final-tags-NNN.jsonl.gz (100000 lines each) and summary.json.
# SPEED: built on WSL-local ext4 ($TC, default ~/kb/scratch/pbsd-tag); the repo shards are re-gzipped (gzip -9, the slow part) only when their content changed since the last run, and
#   /mnt/c files are copied once (publish/cp) instead of being written line by line. final-tags.jsonl, summary and the shard CONTENTS are identical to the previous version (a shard is rewritten only when its content changed, so no gzip-header churn).
. "$(dirname "$0")/lc.sh"
D=/mnt/c/ParanoidBSD/mined-data; F=${F:-$D/final}; R=${R:-/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/final-tags}; mkdir -p "$R"
TC=${TC:-$HOME/kb/scratch/pbsd-tag}; mkdir -p "$TC/shards"; PUBD=$TC; ST=$(state_tsv)
awk -F'\t' '
  function esc(s) { gsub(/\\/, "\\\\", s); gsub(/"/, "\\\"", s); return s }
  { st = $5; r = $6
    if (st == "kept-verified") { s = "verified"; o = "kept as-is" }
    else if (st == "kept-unchecked") { s = "unchecked"; o = "kept as-is" }
    else if (st == "fixed-agree") { s = "verified"; o = "fixed in round " r }
    else if (st == "fixed-unreviewed") { s = "unchecked"; o = "fixed in round " r ", no review verdict" }
    else if (st == "fixed-disagree") { s = "rejected"; o = "reviewer still disagrees after round " r }
    else if (st == "still-rejected") { s = "rejected"; o = "a check still fails after round " r }
    else if (st == "rejected-unrepaired") { s = "rejected"; o = "never repaired" }
    else if (st == "dropped") { s = "dropped"; o = "dropped in round " r }
    else { s = "unchecked"; o = st }
    printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"kind\":\"%s\",\"status\":\"%s\",\"origin\":\"%s\",\"reason\":\"%s\"}\n", esc($1), $2, esc($3), esc($4), s, o, esc($7) }' "$ST" > "$TC/final-tags.jsonl"
awk -F'\t' '{ st = $5; if (st == "kept-verified" || st == "fixed-agree") s = "verified"; else if (st == "kept-unchecked" || st == "fixed-unreviewed") s = "unchecked"; else if (st == "dropped") s = "dropped"; else s = "rejected"
               n++; c[s]++; o[st " r" $6]++; k[$4 " " s]++ }
     END { printf "{\"functions\": %d, \"status\": {", n; m = 0; for (s in c) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, c[s]
           printf "}, \"detail\": {"; m = 0; for (s in o) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, o[s]
           printf "}, \"by_kind\": {"; m = 0; for (s in k) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, k[s]; print "}}" }' "$ST" | jq . > "$TC/final-summary.json"
# repo shards of 100000 lines: only a shard whose content differs from the one already in the repo is re-compressed and rewritten (a changed state touches few shards' worth of lines)
rm -f "$TC"/shards/part.*; split -l 100000 -d -a 3 "$TC/final-tags.jsonl" "$TC/shards/part."
nw=0; for p in "$TC"/shards/part.*; do n=${p##*.}; g=$R/final-tags-$n.jsonl.gz
  if [ -s "$g" ] && zcat "$g" 2>/dev/null | cmp -s - "$p"; then continue; fi
  gzip -9 -n -c "$p" > "$TC/shards/$n.gz" && cp "$TC/shards/$n.gz" "$g.part" && mv "$g.part" "$g"; nw=$((nw+1))
done
for g in "$R"/final-tags-*.jsonl.gz; do n=${g##*final-tags-}; n=${n%.jsonl.gz}; [ -e "$TC/shards/part.$n" ] || rm -f "$g"; done
echo "repo shards rewritten: $nw"
publish "$TC/final-summary.json" "$F/final-summary.json"; publish "$TC/final-tags.jsonl" "$F/final-tags.jsonl"
cp "$TC/final-summary.json" "$R/summary.json"; cp "$F/state-summary.json" "$R/state-summary.json"
jq -c '{functions, status}' "$TC/final-summary.json"
