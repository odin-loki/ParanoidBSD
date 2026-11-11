#!/bin/bash
# mine_retry.sh   Retry every chunk in mined-data/failed.txt with a larger starting token budget (the usual cause of failure was a truncated reply).
# Output goes to mined-data/retry/ (its own ledger, failed list and errors, so the main data is untouched): retry/retry.jsonl.
# Keys look like  tierC:/path/to/file.c:1-267 ; paths contain no colon.
HERE=$(cd "$(dirname "$0")" && pwd); D=/mnt/c/ParanoidBSD/mined-data; R=$D/retry; mkdir -p "$R"
OUT=$R/retry.jsonl
sort -u "$D/failed.txt" | awk -F: '{split($3,a,"-"); print $1, $2, a[1], a[2]}' > "$R/todo.txt"
echo "$(date '+%F %T') retrying $(wc -l < "$R/todo.txt") chunks" | tee -a "$R/retry.log"
# skip chunks already retried successfully (resume)
if [ -s "$OUT" ]; then jq -r '.key' "$OUT" | sort -u > "$R/done.keys"; else : > "$R/done.keys"; fi
while read -r corpus file s e; do
  grep -qxF "$corpus:$file:$s-$e" "$R/done.keys" || echo "$corpus $file $s $e"
done < "$R/todo.txt" > "$R/todo.now"
echo "to do now: $(wc -l < "$R/todo.now")" | tee -a "$R/retry.log"
MINE_MAXTOK=7000 xargs -P 16 -L 1 bash -c "$HERE/mine_chunk.sh \$0 \$1 \$2 \$3 $OUT" < "$R/todo.now" 2>> "$R/retry.err"
echo "$(date '+%F %T') retry finished: results $(wc -l < "$OUT" 2>/dev/null || echo 0), still failed $(sort -u "$R/failed.txt" 2>/dev/null | wc -l)" | tee -a "$R/retry.log"
touch "$R/DONE"
