#!/bin/bash
# mine_sync.sh [interval-s]   Every interval: gzip each COMPLETE block of 4000 lines of the raw mined data into docs/plan/lib/code-mined/shards/ (the data is
# append-only, so a complete block never changes), then commit and push. Stops when a DONE-tier* file exists and everything is synced. (Rule zero: save work as you go.)
R=/c/ParanoidBSD/dreamy-mahavira-7e0e79; D=/c/ParanoidBSD/mined-data; S=$R/docs/plan/lib/code-mined/shards; I=${1:-300}
mkdir -p "$S"; cd "$R" || exit 1
while true; do
  FINAL=""
  [ -f "$D/DONE-all" ] && FINAL=1
  for f in "$D"/tier*.jsonl; do
    [ -f "$f" ] || continue
    b=$(basename "$f" .jsonl); n=$(wc -l < "$f"); full=$((n/4000))
    if [ -n "$FINAL" ] && [ $((n%4000)) -gt 0 ]; then full=$((full+1)); fi
    k=0
    while [ "$k" -lt "$full" ]; do
      t=$S/$b-$(printf %04d "$k").jsonl.gz
      [ -f "$t" ] || sed -n "$((k*4000+1)),$((k*4000+4000))p" "$f" | gzip -9 > "$t"
      k=$((k+1))
    done
  done
  git add docs/plan/lib/code-mined >/dev/null 2>&1
  if ! git diff --cached --quiet; then
    git commit -qm "data: mined code shards ($(ls "$S" | wc -l) shards, $(date +%H:%M))

Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>" && git push -q origin HEAD 2>&1 | tail -1
  fi
  [ -n "$FINAL" ] && break
  sleep "$I"
done
