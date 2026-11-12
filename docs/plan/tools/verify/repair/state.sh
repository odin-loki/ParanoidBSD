#!/bin/bash
# state.sh   The one per-function status table across the original tags and every repair round. Regenerates all counts by script (nothing typed by hand).
# Inputs : mined-data/verify/tags.jsonl (original tags, incl. step 4 + tiebreak), mined-data/verify/repair/{problems.tsv rebuilt below} (why originals were rejected),
#          one directory per repair round k (k=1: verify-repair, k>=2: verify-repair<k>) holding rows.tsv, repair-tags.jsonl, step12.jsonl, frama12.jsonl, s3/s3.jsonl, xcheck/xcheck.jsonl.
# Output : mined-data/final/state.tsv   key  i  name  kind  status  round  reason
#   status: kept-verified | kept-unchecked        original tag, never repaired (kept as-is)
#           rejected-unrepaired                   original tag "rejected", no repair attempted yet (or the repair call failed)
#           dropped                               the repair model said it is not a real function (round = which)
#           still-rejected                        repaired, but a mechanical check still fails (reason = the check)
#           fixed-agree | fixed-disagree | fixed-unreviewed   repaired, passes the mechanical checks, and the DeepSeek review agrees / disagrees (reason) / has no verdict yet
#         The latest round that touched a function decides its status.
# Also: mined-data/final/state-summary.json (counts by status and round), written by this script.
D=/mnt/c/ParanoidBSD/mined-data; V=$D/verify; F=$D/final; mkdir -p "$F/ev"
T=$(printf '\t')
# 1. why the originals were rejected (only needs the original data; rebuilt once)
if [ ! -s "$F/origrepair/problems.tsv" ]; then
  RP=$F/origrepair bash "$(dirname "$0")/repair_build.sh" > "$F/origrepair.log" 2>&1
fi
jq -r '"\(.key)\t\(.i)\t\(.name)\t\(.kind)\t\(.status)"' "$V/tags.jsonl" > "$F/ev/orig.tsv"
: > "$F/ev/events.tsv"
k=1
while true; do
  if [ $k -eq 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$k; fi
  [ -s "$W/repair-tags.jsonl" ] || break
  E=$F/ev/r$k; mkdir -p "$E"
  jq -r '"\(.key)\t\(.i)\t\(.status)"' "$W/repair-tags.jsonl" > "$E/tags.tsv"
  # mechanical reasons of this round's still-rejected functions
  { jq -r 'select(.s1=="fail" or .s2=="fail") | "\(.key)\t\(.i)\t" + ([(if .s1=="fail" then "mechanical: " + .s1_why else empty end), (if .s2=="fail" then "syntax: " + .s2_why else empty end)] | join(" | "))' "$W/step12.jsonl"
    jq -r 'select(.frama=="fail" or (.frama=="type" and (.msg|test("𝔹")))) | "\(.key)\t\(.i)\tFrama-C rejects the formal contract: \(.msg|.[0:120])"' "$W/frama12.jsonl"
    [ -s "$W/s3/s3.jsonl" ] && awk -F'\t' '$3 ~ /^counterexample/ || $3 == "crash" { print $1 "\t" $2 "\trandom testing: " (($3 == "crash") ? "the function traps on an input that meets the precondition" : "contract is FALSE for " $4) }' <(LC_ALL=C awk -f "$(dirname "$0")/s3_tsv.awk" "$W/s3/s3.jsonl" | sed 's/S3-COUNTEREXAMPLE //')
  } | awk -F'\t' '{ k = $1 SUBSEP $2; p[k] = (k in p) ? p[k] " | " $3 : $3 } END { for (k in p) { split(k, a, SUBSEP); print a[1] "\t" a[2] "\t" p[k] } }' > "$E/mech.tsv"
  # review verdicts by (key, name) -> (key, i); a disagreement wins over an agreement for the same name
  if [ -s "$W/xcheck/xcheck.jsonl" ]; then
    jq -r '.key as $k | .verdicts[] | "\($k)\t\(.name)\t\(.verdict)\t\(.reason|gsub("[\t\n\r]";" "))"' "$W/xcheck/xcheck.jsonl" > "$E/x_byname.tsv"
  else : > "$E/x_byname.tsv"; fi
  awk -F'\t' 'FILENAME == ARGV[1] { k = $1 SUBSEP $2; if (!(k in v) || $3 == "disagree") { v[k] = $3; r[k] = $4 } next }
              (($1 SUBSEP $6) in v) { print $1 "\t" $5 "\t" v[$1 SUBSEP $6] "\t" r[$1 SUBSEP $6] }' "$E/x_byname.tsv" "$W/rows.tsv" > "$E/x.tsv"
  awk -F'\t' -v k=$k -v OFS='\t' 'FILENAME == ARGV[1] { mech[$1 SUBSEP $2] = $3; next } FILENAME == ARGV[2] { xv[$1 SUBSEP $2] = $3; xr[$1 SUBSEP $2] = $4; next }
       { q = $1 SUBSEP $2; st = $3; why = ""
         if (st == "dropped") { st = "dropped" }
         else if (st == "still-rejected") { why = (q in mech) ? mech[q] : "a check fails"; st = "still-rejected" }
         else if (st == "fixed") { if (!(q in xv)) st = "fixed-unreviewed"; else if (xv[q] == "agree") st = "fixed-agree"; else { st = "fixed-disagree"; why = "second reviewer: " xr[q] } }
         print $1, $2, k, st, why }' "$E/mech.tsv" "$E/x.tsv" "$E/tags.tsv" >> "$F/ev/events.tsv"
  k=$((k+1))
done
NR_ROUNDS=$((k-1))
# latest round wins (events are appended in round order)
awk -F'\t' '{ ev[$1 SUBSEP $2] = $3 "\t" $4 "\t" $5 }
  END { for (q in ev) { split(q, a, SUBSEP); print a[1] "\t" a[2] "\t" ev[q] } }' "$F/ev/events.tsv" > "$F/ev/latest.tsv"
awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { pr[$1 SUBSEP $2] = $3; next } FILENAME == ARGV[2] { ev[$1 SUBSEP $2] = $3 "\t" $4 "\t" $5; next }
  { q = $1 SUBSEP $2
    if (q in ev) { split(ev[q], e, "\t"); print $1, $2, $3, $4, e[2], e[1], e[3] }
    else if ($5 == "rejected") print $1, $2, $3, $4, "rejected-unrepaired", 0, ((q in pr) ? pr[q] : "rejected (see tags)")
    else print $1, $2, $3, $4, "kept-" $5, 0, "" }' "$F/origrepair/problems.tsv" "$F/ev/latest.tsv" "$F/ev/orig.tsv" > "$F/state.tsv.part"
mv "$F/state.tsv.part" "$F/state.tsv"
awk -F'\t' '{ c[$5 " r" $6]++; t[$5]++ } END { printf "{\"rounds\": '"$NR_ROUNDS"', \"by_status\": {"; n = 0; for (s in t) printf "%s\"%s\": %d", (n++ ? ", " : ""), s, t[s]; printf "}, \"by_status_round\": {"; n = 0; for (s in c) printf "%s\"%s\": %d", (n++ ? ", " : ""), s, c[s]; print "}}" }' "$F/state.tsv" | jq . > "$F/state-summary.json"
cat "$F/state-summary.json"
