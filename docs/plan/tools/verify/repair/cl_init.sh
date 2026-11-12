#!/bin/bash
# cl_init.sh   Snapshot the DeepSeek stage's queue into mined-data/claude (read-only use of mined-data/dsv): work.jsonl copy (stable line numbers for the batch runner),
# groups.tsv = "key<TAB>line<TAB>group" (0 still-rejected, 1 fixed-disagree, 2 fixed-unreviewed, 3 rejected-unrepaired, 4 dropped-only), order.tsv copy.
CLD=/mnt/c/ParanoidBSD/mined-data/claude; DW=/mnt/c/ParanoidBSD/mined-data/dsv
mkdir -p "$CLD/jobs"
[ -s "$CLD/work.jsonl" ] || { cp "$DW/work.jsonl" "$CLD/work.jsonl.part" && mv "$CLD/work.jsonl.part" "$CLD/work.jsonl"; cp "$DW/order.tsv" "$CLD/order.tsv"; }
[ -s "$CLD/groups.tsv" ] || {
  jq -r '[.key, ([.functions[].status | if .=="still-rejected" then 0 elif .=="fixed-disagree" then 1 elif .=="fixed-unreviewed" then 2 elif .=="rejected-unrepaired" then 3 else 4 end] | min)] | @tsv' "$CLD/work.jsonl" |
    awk -F'\t' -v OFS='\t' '{ print $1, NR, $2 }' > "$CLD/groups.tsv.part" && mv "$CLD/groups.tsv.part" "$CLD/groups.tsv"; }
echo "records $(wc -l < "$CLD/work.jsonl")"; cut -f3 "$CLD/groups.tsv" | sort | uniq -c
