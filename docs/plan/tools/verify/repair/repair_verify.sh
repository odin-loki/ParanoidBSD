#!/bin/bash
# repair_verify.sh   Re-verify the repaired annotations with the SAME free checks as the originals: steps 1-2 (verify12.awk), Frama-C (run_frama.sh), step 3 (run_s3.sh).
# Works in its own directory (mined-data/verify-repair, scratch in ~/verify-scratch-$SFX and ~/s3-scratch-$SFX), so the original results are untouched.
# Output: mined-data/verify-repair/repair-tags.jsonl, one line per repaired function:
#   {"key","i","name","action","after":{"mech","syntax","fuzz"},"status":"fixed|still-rejected|dropped|unchecked"}
#   fixed = every check that applies passes (mechanical pass; a formal contract parses in Frama-C and the stub grammar; no step 3 counterexample)
# The step 4 cross-check is not re-run here (it is paid); repaired functions that were rejected only by DeepSeek are "unchecked" until it is.
. "$(dirname "$0")/lc.sh"; H=$(cd "$(dirname "$0")/.." && pwd); V=/mnt/c/ParanoidBSD/mined-data/verify; W=${W:-/mnt/c/ParanoidBSD/mined-data/verify-repair}; RJ=${RJ:-$V/repair/repair.jsonl}; SFX=${SFX:-repair}   # W: output dir, RJ: repair.jsonl to verify, SFX: scratch name
[ -n "$TAGS_ONLY" ] || rm -rf "$W"; mkdir -p "$W"   # TAGS_ONLY=1: skip the checks, recompute only t_*.tsv and repair-tags.jsonl from the results already in $W
if [ -z "$TAGS_ONLY" ]; then
# rows in the verify12 format: key file start end idx name kind contract prose  (kind taken from the original row)
jq -r '. as $r | $r.result.functions[] | select(.action == "fix") | [$r.key, $r.file, ($r.start|tostring), ($r.end|tostring), (.idx|tostring),
       (.name // "" | gsub("[\t\n\r]";" ")), "", (.contract // "" | gsub("[\t\n\r]";" ")), (.prose_contract // "" | gsub("[\t\n\r]";" "))] | join("\t")' "$RJ" |
  awk -F'\t' -v OFS='\t' 'FILENAME == ARGV[1] { kind[$1 SUBSEP $5] = $7; next } { $7 = kind[$1 SUBSEP $5]; print }' "$(lc "$V/rows.tsv")" - > "$W/rows.tsv"
jq -r '. as $r | $r.result.functions[] | select(.action == "drop") | "\($r.key)\t\(.idx)\t\(.name // "")"' "$RJ" > "$W/dropped.tsv"
echo "repaired rows: $(wc -l < "$W/rows.tsv"), dropped: $(wc -l < "$W/dropped.tsv")"
VERIFY_DIR=$W bash "$H/run_verify12.sh" > /dev/null 2>&1
VERIFY_DIR=$W SCRATCH_DIR=$HOME/verify-scratch-$SFX bash "$H/run_frama.sh" > /dev/null 2>&1
VERIFY_DIR=$W SCRATCH_DIR=$HOME/s3-scratch-$SFX bash "$H/s3/run_s3.sh" > /dev/null 2>&1
fi
mkdir -p "$W/s3"; [ -f "$W/s3/s3.jsonl" ] || : > "$W/s3/s3.jsonl"
jq -r '"\(.key)\t\(.i)\t\(.frama)\t\(.msg)"' "$W/frama12.jsonl" > "$W/t_frama.tsv" 2>/dev/null
LC_ALL=C awk -f "$(dirname "$0")/s3_tsv.awk" "$W/s3/s3.jsonl" | cut -f1-3 > "$W/t_s3.tsv"   # not jq: see s3_tsv.awk
{ jq -r '"\(.key)\t\(.i)\t\(.name)\t\(.has_contract)\t\(.s1)\t\(.s2)"' "$W/step12.jsonl"; } |
awk -F'\t' -v fr="$W/t_frama.tsv" -v s3f="$W/t_s3.tsv" '
  BEGIN { while ((getline l < fr) > 0) { split(l, a, "\t"); v = a[3]; if (v == "type" && a[4] ~ /𝔹/) v = "fail"; if (v == "fail" && a[4] ~ /errno as external/) v = "unknown"; F[a[1] SUBSEP a[2]] = v }
          while ((getline l < s3f) > 0) { split(l, a, "\t"); S3[a[1] SUBSEP a[2]] = a[3] } }
  { k = $1 SUBSEP $2; mech = $5
    if ($4 != "true") syntax = "skip"; else { f = (k in F) ? F[k] : ""; syntax = ($6 == "fail" || f == "fail") ? "fail" : (f == "pass" ? "pass" : "skip") }
    t = (k in S3) ? S3[k] : ""; fuzz = (t == "pass") ? "pass" : ((t ~ /^counterexample/ || t == "crash") ? "fail" : "skip")
    st = (mech == "fail" || syntax == "fail" || fuzz == "fail") ? "still-rejected" : "fixed"
    key = $1; name = $3; gsub(/\\/, "\\\\", key); gsub(/"/, "\\\"", key); gsub(/\\/, "\\\\", name); gsub(/"/, "\\\"", name)
    printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"action\":\"fix\",\"after\":{\"mech\":\"%s\",\"syntax\":\"%s\",\"fuzz\":\"%s\"},\"status\":\"%s\"}\n", key, $2, name, mech, syntax, fuzz, st }' > "$W/repair-tags.jsonl"
awk -F'\t' '{ k = $1; n = $3; gsub(/\\/, "\\\\", k); gsub(/"/, "\\\"", k); gsub(/\\/, "\\\\", n); gsub(/"/, "\\\"", n); printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"action\":\"drop\",\"status\":\"dropped\"}\n", k, $2, n }' "$W/dropped.tsv" >> "$W/repair-tags.jsonl"
jq -s '{functions: length, status: (group_by(.status)|map({(.[0].status): length})|add),
        still_rejected_because: ([.[]|select(.status=="still-rejected")|.after|to_entries[]|select(.value=="fail")|.key]|group_by(.)|map({(.[0]): length})|add)}' "$W/repair-tags.jsonl" | tee "$W/summary.json"
