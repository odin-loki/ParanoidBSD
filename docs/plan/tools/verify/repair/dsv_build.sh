#!/bin/bash
# dsv_build.sh   Snapshot the manual packets (export_manual.sh deletes and regenerates them) into mined-data/dsv/work.jsonl, skipping the hand-fixed packets (a results file exists in
# manual/results/) and the packets named in $SKIP. One record per line; order.tsv = "key<TAB>line" in priority order: records with a not-yet-dropped function first, dropped-only last.
DW=/mnt/c/ParanoidBSD/mined-data/dsv; M=/mnt/c/ParanoidBSD/mined-data/manual; mkdir -p "$DW"
: > "$DW/work.jsonl"
for p in "$M"/packets/packet-P*.jsonl; do
  id=$(basename "$p" .jsonl); id=${id#packet-}
  [ -s "$M/results/$id.jsonl" ] && continue
  case " $SKIP " in *" $id "*) continue;; esac
  cat "$p" >> "$DW/work.jsonl"
done
jq -r '[.key, ([.functions[].status | if .=="still-rejected" then 0 elif .=="fixed-disagree" then 1 elif .=="fixed-unreviewed" then 2 elif .=="rejected-unrepaired" then 3 else 4 end] | min)] | @tsv' "$DW/work.jsonl" | awk -F'\t' -v OFS='\t' '{ print $1, NR, $2 }' | sort -s -t "$(printf '\t')" -k3,3n -k2,2n | cut -f1,2 > "$DW/order.tsv"
echo "records $(wc -l < "$DW/work.jsonl"), functions $(jq '.functions|length' "$DW/work.jsonl" | awk '{s+=$1} END{print s}')"
