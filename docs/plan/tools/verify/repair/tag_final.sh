#!/bin/bash
# tag_final.sh   Re-tag ALL mined functions after the repair rounds, from final/state.tsv (run state.sh first). Every count comes from this script.
# Output: mined-data/final/final-tags.jsonl, one line per function:
#   {"key","i","name","kind","status":"verified|unchecked|rejected|dropped","origin":"kept as-is|fixed in round N|dropped in round N|...","reason"}
#   status verified    = original tag verified (kept as-is), or repaired and every check passes and the DeepSeek review agrees
#   status unchecked   = original tag unchecked (kept as-is), or repaired but the review has no verdict (failed call)
#   status rejected    = still rejected after the last round it went through (reason given), or never repaired
#   status dropped     = the repair model said it is not a real function (listed, not hidden)
# Repo copy: docs/plan/lib/code-mined/final-tags/final-tags-NNN.jsonl.gz (100000 lines each) and summary.json.
D=/mnt/c/ParanoidBSD/mined-data; F=$D/final; R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/final-tags; mkdir -p "$R"
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
    printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"kind\":\"%s\",\"status\":\"%s\",\"origin\":\"%s\",\"reason\":\"%s\"}\n", esc($1), $2, esc($3), esc($4), s, o, esc($7) }' "$F/state.tsv" > "$F/final-tags.jsonl.part"
mv "$F/final-tags.jsonl.part" "$F/final-tags.jsonl"
awk -F'\t' '{ st = $5; if (st == "kept-verified" || st == "fixed-agree") s = "verified"; else if (st == "kept-unchecked" || st == "fixed-unreviewed") s = "unchecked"; else if (st == "dropped") s = "dropped"; else s = "rejected"
               n++; c[s]++; o[st " r" $6]++; k[$4 " " s]++ }
     END { printf "{\"functions\": %d, \"status\": {", n; m = 0; for (s in c) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, c[s]
           printf "}, \"detail\": {"; m = 0; for (s in o) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, o[s]
           printf "}, \"by_kind\": {"; m = 0; for (s in k) printf "%s\"%s\": %d", (m++ ? ", " : ""), s, k[s]; print "}}" }' "$F/state.tsv" | jq . > "$F/final-summary.json"
rm -f "$R"/final-tags-*.jsonl.gz
split -l 100000 -d -a 3 "$F/final-tags.jsonl" "$F/final-tags.part."
for p in "$F"/final-tags.part.*; do gzip -9 -c "$p" > "$R/final-tags-${p##*.}.jsonl.gz"; rm -f "$p"; done
cp "$F/final-summary.json" "$R/summary.json"; cp "$F/state-summary.json" "$R/state-summary.json"
jq -c '{functions, status}' "$F/final-summary.json"
