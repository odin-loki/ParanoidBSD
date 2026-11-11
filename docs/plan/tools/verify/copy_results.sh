V=/mnt/c/ParanoidBSD/mined-data/verify; R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/verify-results; mkdir -p $R/tiebreak
cp $V/summary.json $R/step12-summary.json; cp $V/frama-summary.json $R/frama-summary.json; cp $V/s3/summary.json $R/s3-summary.json
gzip -9 -c $V/s3/s3.jsonl > $R/s3.jsonl.gz; gzip -9 -c $V/frama12.jsonl > $R/frama12.jsonl.gz
cp $V/tiebreak/index.tsv $V/tiebreak/verdicts-*.tsv $R/tiebreak/; tar -czf $R/tiebreak/packets.tar.gz -C $V/tiebreak packets
# evidence level among verified functions
jq -r 'select(.status=="verified") | [ (.verified.fuzz=="pass"), (.verified.xcheck=="agree" or .verified.tiebreak=="overturn"), (.verified.syntax=="pass") ] | if .[0] then "fuzz-tested" elif .[1] then "cross-checked" else "syntax only" end' $V/tags.jsonl | sort | uniq -c
du -sh $R | cut -f1
