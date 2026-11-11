#!/bin/bash
# tag_all.sh   Step 6 of VERIFICATION_PLAN.md: one tag line per mined function, combining every step. The original shards are not touched.
# Inputs (mined-data/verify): step12.jsonl (steps 1-2), frama12.jsonl (Frama-C parse), s3/s3.jsonl (step 3), xcheck/xcheck.jsonl (step 4), tiebreak/verdicts-*.tsv
# Output: mined-data/verify/tags.jsonl (and gzip shards in the repo: docs/plan/lib/code-mined/tags/), summary in tags-summary.json.
# Per function:
#   mech   = step 1 (pass|fail|skip)
#   syntax = fail if the stub grammar or Frama-C rejects the contract; pass if Frama-C accepts it; skip if there is no contract or Frama-C could not judge it
#            Reclassified from Frama-C's raw verdicts: "unexpected 𝔹" type errors are precedence bugs (fail); "errno as external identifier" is a stub artefact (skip)
#   fuzz   = step 3: pass | fail (counterexample or a trap under the precondition) | skip
#   xcheck = step 4: agree | disagree | skip ; tiebreak = uphold | overturn | unclear when a sampled disagreement was reviewed
#   verified = true when nothing failed or disagreed (an upheld disagreement counts as a disagreement) and at least one check passed or agreed
V=/mnt/c/ParanoidBSD/mined-data/verify; R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/tags; mkdir -p "$R"
jq -r '"\(.key)\t\(.i)\t\(.frama)\t\(.msg)"' "$V/frama12.jsonl" > "$V/t_frama.tsv"
[ -s "$V/s3/s3.jsonl" ] && jq -r '"\(.key)\t\(.i)\t\(.s3)"' "$V/s3/s3.jsonl" > "$V/t_s3.tsv" || : > "$V/t_s3.tsv"
[ -s "$V/xcheck/xcheck.jsonl" ] && jq -r '.key as $k | .verdicts[] | "\($k)\t\(.name)\t\(.verdict)"' "$V/xcheck/xcheck.jsonl" > "$V/t_x.tsv" || : > "$V/t_x.tsv"
cat "$V"/tiebreak/verdicts-*.tsv 2>/dev/null | awk -F'\t' 'FILENAME == ARGV[1] { k[$1] = $2 "\t" $3; next } ($1 in k) { print k[$1] "\t" tolower($2) }' "$V/tiebreak/index.tsv" - > "$V/t_tb.tsv" 2>/dev/null || : > "$V/t_tb.tsv"
jq -r '"\(.key)\t\(.i)\t\(.name)\t\(.kind)\t\(.has_contract)\t\(.s1)\t\(.s2)"' "$V/step12.jsonl" |
awk -F'\t' -v fr="$V/t_frama.tsv" -v s3f="$V/t_s3.tsv" -v xf="$V/t_x.tsv" -v tbf="$V/t_tb.tsv" '
  BEGIN {
    while ((getline l < fr) > 0) { split(l, a, "\t"); v = a[3]; m = a[4]
      if (v == "type" && m ~ /unexpected 𝔹|𝔹 and/) v = "fail"
      if (v == "fail" && m ~ /errno as external/) v = "unknown"
      F[a[1] SUBSEP a[2]] = v }
    while ((getline l < s3f) > 0) { split(l, a, "\t"); S3[a[1] SUBSEP a[2]] = a[3] }
    while ((getline l < xf) > 0) { split(l, a, "\t"); X[a[1] SUBSEP a[2]] = a[3] }
    while ((getline l < tbf) > 0) { split(l, a, "\t"); TB[a[1] SUBSEP a[2]] = a[3] }
  }
  {
    key = $1; i = $2; name = $3; kind = $4; hc = $5; s1 = $6; s2 = $7; k = key SUBSEP i; kn = key SUBSEP name
    mech = s1
    if (hc != "true") syntax = "skip"
    else { f = (k in F) ? F[k] : ""; if (s2 == "fail" || f == "fail") syntax = "fail"; else if (f == "pass") syntax = "pass"; else syntax = "skip" }
    t = (k in S3) ? S3[k] : ""; fuzz = (t == "pass") ? "pass" : ((t ~ /^counterexample/ || t == "crash") ? "fail" : "skip")
    fnote = (t == "counterexample-limits") ? ",\"fuzz_note\":\"wrong only at integer limits\"" : ((t == "counterexample-logic") ? ",\"fuzz_note\":\"wrong for ordinary values\"" : ((t == "crash") ? ",\"fuzz_note\":\"traps under its precondition\"" : ""))
    xc = (kn in X) ? X[kn] : "skip"; tb = (kn in TB) ? TB[kn] : ""
    bad = (mech == "fail" || syntax == "fail" || fuzz == "fail" || (xc == "disagree" && tb != "overturn"))
    good = (syntax == "pass" || fuzz == "pass" || xc == "agree" || (xc == "disagree" && tb == "overturn"))
    gsub(/\\/, "\\\\", key); gsub(/"/, "\\\"", key); gsub(/\\/, "\\\\", name); gsub(/"/, "\\\"", name)
    printf "{\"key\":\"%s\",\"i\":%s,\"name\":\"%s\",\"kind\":\"%s\",\"verified\":{\"mech\":\"%s\",\"syntax\":\"%s\",\"fuzz\":\"%s\",\"xcheck\":\"%s\"%s},\"status\":\"%s\"}\n", \
      key, i, name, kind, mech, syntax, fuzz, xc, (tb != "" ? ",\"tiebreak\":\"" tb "\"" : "") fnote, (bad ? "rejected" : (good ? "verified" : "unchecked"))
  }' > "$V/tags.jsonl"
jq -s '{functions: length, status: (group_by(.status)|map({(.[0].status): length})|add),
        syntax: (group_by(.verified.syntax)|map({(.[0].verified.syntax): length})|add), fuzz: (group_by(.verified.fuzz)|map({(.[0].verified.fuzz): length})|add),
        xcheck: (group_by(.verified.xcheck)|map({(.[0].verified.xcheck): length})|add),
        verified_by_kind: ([.[]|select(.status=="verified")]|group_by(.kind)|map({(.[0].kind): length})|add)}' "$V/tags.jsonl" > "$V/tags-summary.json"
# repo copy: gzip shards of 100000 lines, plus the summary
rm -f "$R"/tags-*.jsonl.gz
split -l 100000 -d -a 3 "$V/tags.jsonl" "$V/tags.part."
for p in "$V"/tags.part.*; do gzip -9 -c "$p" > "$R/tags-${p##*.}.jsonl.gz"; rm -f "$p"; done
cp "$V/tags-summary.json" "$R/summary.json"
cat "$V/tags-summary.json"
