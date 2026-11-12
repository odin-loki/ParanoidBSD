#!/bin/bash
# state.sh   The one per-function status table across the original tags and every repair round. Regenerates all counts by script (nothing typed by hand).
# Inputs : mined-data/verify/tags.jsonl (original tags, incl. step 4 + tiebreak), mined-data/final/origrepair/problems.tsv (why originals were rejected, built once below),
#          one directory per repair round k (k=1: verify-repair, k>=2: verify-repair<k>) holding rows.tsv, repair-tags.jsonl, step12.jsonl, frama12.jsonl, s3/s3.jsonl, xcheck/xcheck.jsonl.
# Output : mined-data/final/state.tsv   key  i  name  kind  status  round  reason
#   status: kept-verified | kept-unchecked        original tag, never repaired (kept as-is)
#           rejected-unrepaired                   original tag "rejected", no repair attempted yet (or the repair call failed)
#           dropped                               the repair model said it is not a real function (round = which)
#           still-rejected                        repaired, but a mechanical check still fails (reason = the check)
#           fixed-agree | fixed-disagree | fixed-unreviewed   repaired, passes the mechanical checks, and the DeepSeek review agrees / disagrees (reason) / has no verdict yet
#         The latest round that touched a function decides its status.
# Also: mined-data/final/state-summary.json (counts by status and round), mined-data/final/ev/events.tsv (one line per function per round), written by this script.
# SPEED (this version): all work is done on WSL-local ext4 in $SC (default ~/kb/scratch/pbsd-state); /mnt/c (9p, slow, shared with 100 parallel jobs) is read for the small per-round inputs
#   and written ONCE per output (state.tsv via atomic mv, and only when its content changed). Per-round events and the original-tags table are cached by input size+mtime
#   (+ a hash of this script), so a new round recomputes only that round; the old script re-ran jq over all 275 MB of tags and every round's files every time.
#   Not written any more (nothing reads them): final/ev/orig.tsv, final/ev/latest.tsv, final/ev/r<k>/ (they live in $SC). SC=<dir> overrides the cache; FORCE=1 ignores the caches; F=<dir> overrides the output dir.
D=/mnt/c/ParanoidBSD/mined-data; V=$D/verify; F=${F:-$D/final}; mkdir -p "$F/ev"
HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/lc.sh"
SC=${SC:-$HOME/kb/scratch/pbsd-state}; mkdir -p "$SC"; exec 8> "$SC/.lock"; flock 8     # one state.sh at a time per cache
SELF=$(md5sum "$0" "$HERE/s3_tsv.awk" | cut -d' ' -f1 | tr '\n' ' ')
sig() { stat -c '%n %s %Y' "$@" 2>/dev/null | tr '\n' ';'; }
# 1. why the originals were rejected (only needs the original data; rebuilt once)
if [ ! -s "$F/origrepair/problems.tsv" ]; then
  RP=$F/origrepair bash "$HERE/repair_build.sh" > "$F/origrepair.log" 2>&1
fi
# 2. original tags -> orig.tsv (cached on tags.jsonl's size+mtime)
OSIG="$(sig "$V/tags.jsonl")"
if [ -n "$FORCE" ] || [ "$(cat "$SC/orig.sig" 2>/dev/null)" != "$OSIG" ] || [ ! -s "$SC/orig.tsv" ]; then
  jq -r '"\(.key)\t\(.i)\t\(.name)\t\(.kind)\t\(.status)"' "$(lc "$V/tags.jsonl")" > "$SC/orig.tsv.part" && mv "$SC/orig.tsv.part" "$SC/orig.tsv" && printf '%s' "$OSIG" > "$SC/orig.sig" || exit 1
fi
PSIG="$(sig "$F/origrepair/problems.tsv")"
if [ "$(cat "$SC/problems.sig" 2>/dev/null)" != "$PSIG" ]; then cp "$F/origrepair/problems.tsv" "$SC/problems.tsv.part" && mv "$SC/problems.tsv.part" "$SC/problems.tsv" && printf '%s' "$PSIG" > "$SC/problems.sig"; fi
# 3. per-round events (cached per round on the round's input files)
round_events() {   # round_events k W E   -> $E/events.part
  local k=$1 W=$2 E=$3 RT S12 FR S3 XC RW; RT=$(lc "$W/repair-tags.jsonl"); S12=$(lc "$W/step12.jsonl"); FR=$(lc "$W/frama12.jsonl"); RW=$(lc "$W/rows.tsv"); [ -s "$W/s3/s3.jsonl" ] && S3=$(lc "$W/s3/s3.jsonl"); [ -s "$W/xcheck/xcheck.jsonl" ] && XC=$(lc "$W/xcheck/xcheck.jsonl")   # 9p is read with tiny blocks by jq/awk: mirror the files first (cp reads big blocks)
  jq -r '"\(.key)\t\(.i)\t\(.status)"' "$RT" > "$E/tags.tsv"
  # mechanical reasons of this round's still-rejected functions
  { jq -r 'select(.s1=="fail" or .s2=="fail") | "\(.key)\t\(.i)\t" + ([(if .s1=="fail" then "mechanical: " + .s1_why else empty end), (if .s2=="fail" then "syntax: " + .s2_why else empty end)] | join(" | "))' "$S12"
    jq -r 'select(.frama=="fail" or (.frama=="type" and (.msg|test("𝔹")))) | "\(.key)\t\(.i)\tFrama-C rejects the formal contract: \(.msg|.[0:120])"' "$FR"
    [ -n "$S3" ] && awk -F'\t' '$3 ~ /^counterexample/ || $3 == "crash" { print $1 "\t" $2 "\trandom testing: " (($3 == "crash") ? "the function traps on an input that meets the precondition" : "contract is FALSE for " $4) }' <(LC_ALL=C awk -f "$HERE/s3_tsv.awk" "$S3" | sed 's/S3-COUNTEREXAMPLE //')
  } | awk -F'\t' '{ k = $1 SUBSEP $2; p[k] = (k in p) ? p[k] " | " $3 : $3 } END { for (k in p) { split(k, a, SUBSEP); print a[1] "\t" a[2] "\t" p[k] } }' > "$E/mech.tsv"
  # review verdicts by (key, name) -> (key, i); a disagreement wins over an agreement for the same name
  if [ -n "$XC" ]; then
    jq -r '.key as $k | .verdicts[] | "\($k)\t\(.name)\t\(.verdict)\t\(.reason|gsub("[\t\n\r]";" "))"' "$XC" > "$E/x_byname.tsv"
  else : > "$E/x_byname.tsv"; fi
  awk -F'\t' 'FILENAME == ARGV[1] { k = $1 SUBSEP $2; if (!(k in v) || $3 == "disagree") { v[k] = $3; r[k] = $4 } next }
              (($1 SUBSEP $6) in v) { print $1 "\t" $5 "\t" v[$1 SUBSEP $6] "\t" r[$1 SUBSEP $6] }' "$E/x_byname.tsv" "$RW" > "$E/x.tsv"
  # non-failing notes of the free checks (s1_note, e.g. "name outside slice": manual results whose name is not in the slice), kept visible on the fixed-* statuses
  jq -r 'select(.s1_note) | "\(.key)\t\(.i)\t\(.s1_note)"' "$S12" > "$E/notes.tsv" 2>/dev/null || : > "$E/notes.tsv"
  awk -F'\t' -v k=$k -v OFS='\t' 'FILENAME == ARGV[1] { mech[$1 SUBSEP $2] = $3; next } FILENAME == ARGV[2] { xv[$1 SUBSEP $2] = $3; xr[$1 SUBSEP $2] = $4; next } FILENAME == ARGV[3] { note[$1 SUBSEP $2] = $3; next }
       { q = $1 SUBSEP $2; st = $3; why = ""
         if (st == "dropped") { st = "dropped" }
         else if (st == "still-rejected") { why = (q in mech) ? mech[q] : "a check fails"; st = "still-rejected" }
         else if (st == "fixed") { if (!(q in xv)) st = "fixed-unreviewed"; else if (xv[q] == "agree") st = "fixed-agree"; else { st = "fixed-disagree"; why = "second reviewer: " xr[q] }
                                   if (q in note) why = (why == "" ? "" : why " | ") "note: " note[q] }
         print $1, $2, k, st, why }' "$E/mech.tsv" "$E/x.tsv" "$E/notes.tsv" "$E/tags.tsv" > "$E/events.part.tmp" && mv "$E/events.part.tmp" "$E/events.part"
}
k=1; EV=""
while true; do
  if [ $k -eq 1 ]; then W=$D/verify-repair; else W=$D/verify-repair$k; fi
  [ -s "$W/repair-tags.jsonl" ] || break
  E=$SC/r$k; mkdir -p "$E"
  RSIG="$SELF $(sig "$W/repair-tags.jsonl" "$W/step12.jsonl" "$W/frama12.jsonl" "$W/s3/s3.jsonl" "$W/xcheck/xcheck.jsonl" "$W/rows.tsv")"
  if [ -n "$FORCE" ] || [ "$(cat "$E/sig" 2>/dev/null)" != "$RSIG" ] || [ ! -f "$E/events.part" ]; then
    rm -f "$E/sig"; round_events $k "$W" "$E" || exit 1; printf '%s' "$RSIG" > "$E/sig"
  fi
  EV="$EV $E/events.part"; k=$((k+1))
done
NR_ROUNDS=$((k-1))
cat $EV > "$SC/events.tsv"; : >> "$SC/events.tsv"
# 4. latest round wins (events are in round order); merge with the originals
awk -F'\t' '{ ev[$1 SUBSEP $2] = $3 "\t" $4 "\t" $5 }
  END { for (q in ev) { split(q, a, SUBSEP); print a[1] "\t" a[2] "\t" ev[q] } }' "$SC/events.tsv" > "$SC/latest.tsv"
awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { pr[$1 SUBSEP $2] = $3; next } FILENAME == ARGV[2] { ev[$1 SUBSEP $2] = $3 "\t" $4 "\t" $5; next }
  { q = $1 SUBSEP $2
    if (q in ev) { split(ev[q], e, "\t"); print $1, $2, $3, $4, e[2], e[1], e[3] }
    else if ($5 == "rejected") print $1, $2, $3, $4, "rejected-unrepaired", 0, ((q in pr) ? pr[q] : "rejected (see tags)")
    else print $1, $2, $3, $4, "kept-" $5, 0, "" }' "$SC/problems.tsv" "$SC/latest.tsv" "$SC/orig.tsv" > "$SC/state.tsv.part"
awk -F'\t' '{ c[$5 " r" $6]++; t[$5]++ } END { printf "{\"rounds\": '"$NR_ROUNDS"', \"by_status\": {"; n = 0; for (s in t) printf "%s\"%s\": %d", (n++ ? ", " : ""), s, t[s]; printf "}, \"by_status_round\": {"; n = 0; for (s in c) printf "%s\"%s\": %d", (n++ ? ", " : ""), s, c[s]; print "}}" }' "$SC/state.tsv.part" | jq . > "$SC/state-summary.json"
# 5. publish: each output is written to /mnt/c once, via a temp file and an atomic mv, and only if it differs from what is there
mv "$SC/state.tsv.part" "$SC/state.tsv"
publish "$SC/events.tsv" "$F/ev/events.tsv"
publish "$SC/state.tsv" "$F/state.tsv"
publish "$SC/state-summary.json" "$F/state-summary.json"
cat "$F/state-summary.json"
