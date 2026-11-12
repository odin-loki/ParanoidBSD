#!/bin/bash
# cl_opus_round.sh [max-records]   AFTER the DeepSeek ingest and export_manual.sh: snapshot the regenerated manual packets (mined-data/manual/packets/, failure history now includes the
# DeepSeek round) as work2.jsonl and create jobs/opus-round2 (Opus 5.5, batch, effort high) with the records ordered still-rejected, fixed-disagree, fixed-unreviewed, rejected-unrepaired,
# dropped-only. It only prepares the job; start it with start_claude.cmd opus-round2. Keys already done by a Claude job are skipped by the job itself.
CLD=/mnt/c/ParanoidBSD/mined-data/claude; M=/mnt/c/ParanoidBSD/mined-data/manual; R=$(cd "$(dirname "$0")" && pwd); MAXN=${1:-100000}
J=$CLD/jobs/opus-round2; mkdir -p "$J"
: > "$CLD/work2.jsonl"
for p in "$M"/packets/packet-P*.jsonl; do id=$(basename "$p" .jsonl); id=${id#packet-}; [ -s "$M/results/$id.jsonl" ] && continue; cat "$p" >> "$CLD/work2.jsonl"; done
jq -r '[.key, ([.functions[].status | if .=="still-rejected" then 0 elif .=="fixed-disagree" then 1 elif .=="fixed-unreviewed" then 2 elif .=="rejected-unrepaired" then 3 else 4 end] | min)] | @tsv' "$CLD/work2.jsonl" |
  awk -F'\t' -v OFS='\t' '{ print $1, NR, $2 }' | sort -s -t "$(printf '\t')" -k3,3n -k2,2n | cut -f1,2 | head -n "$MAXN" > "$J/cand.tsv"
cat > "$J/job.env" <<EOT
# Opus 5.5 round 2: the records still failing after the DeepSeek stage (work2.jsonl = packets after export_manual.sh), in status priority order.
KIND=repair
MODEL=claude-opus-5-5
IN=$CLD/work2.jsonl
CANDS="cat $J/cand.tsv"
BSZ=100
MAXINFL=2
CAP=150
EST_OUT=2500
EOT
echo "opus-round2: $(wc -l < "$J/cand.tsv") records, groups: $(cut -f2 "$J/cand.tsv" | wc -l) ; start: start_claude.cmd opus-round2"
