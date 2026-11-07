#!/bin/bash
# mine_all.sh <tier: B|C|all> [budget-usd]   Mine whole corpora with GLM-5.3-Flash in batches of 400 chunks, resumable, with a spend cap and automatic back-off.
# Raw output: /mnt/c/ParanoidBSD/mined-data/<tier>.jsonl (a persistent Windows folder OUTSIDE the repo: about 1.5 KB per chunk). mine_sync.sh commits gzip shards into the repo.
#   Tier B: core code (Linux kernel mm fs net block lib ipc security crypto init; HBSD sys/{kern,vm,net*,fs,ufs,geom,libkern,security,crypto}; all of pbsd and kde)
#   Tier C: everything else (drivers, arch, tools, ...).   Cost model: Morph prices, input $0.047/M, output $0.268/M tokens (upper bound: cached input is cheaper).
# Env: MINE_P parallel calls (default 12, adapts between 4 and 32).
HERE=$(cd "$(dirname "$0")" && pwd); R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79; D=/mnt/c/ParanoidBSD/mined-data; L=$HOME/pbsd-ref/linux
TIER=${1:-B}; BUDGET=${2:-50}; P=${MINE_P:-12}; mkdir -p "$D"; OUT=$D/tier$TIER.jsonl; LIST=$D/chunks-tier$TIER.txt

cost() { cat "$D"/*.jsonl 2>/dev/null | jq -s '[.[]|(.usage.prompt_tokens*0.047+.usage.completion_tokens*0.268)/1000000]|add // 0'; }

b_files() {
  for d in kernel mm fs net block lib ipc security crypto init; do find "$L/$d" -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null; done
  for d in kern vm net netinet netinet6 netpfil fs ufs geom libkern security crypto sys; do find "$R/hbsd/src/sys/$d" -type f \( -name '*.c' -o -name '*.h' \) 2>/dev/null; done
  find "$R/pbsd" "$R/kde" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null
}
all_files() {
  find "$L" "$R/hbsd" "$R/pbsd" "$R/kde" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null
}
chunker() {
  while read -r f; do
    awk -v f="$f" 'BEGIN{s=1;n=0} {n++} (n-s>=250 && $0=="") || (n-s>=300){print f, s, n; s=n+1} END{if(n>=s&&n<=20000)print f, s, n}' "$f"
  done
}

if [ ! -s "$LIST" ]; then
  case $TIER in
    B) b_files | sort -u | chunker | shuf --random-source=<(yes 7) > "$LIST" ;;
    C) comm -23 <(all_files | sort -u) <(b_files | sort -u) | chunker | shuf --random-source=<(yes 7) > "$LIST" ;;
    all) all_files | sort -u | chunker | shuf --random-source=<(yes 7) > "$LIST" ;;
  esac
fi
TOTAL=$(wc -l < "$LIST"); echo "tier $TIER: $TOTAL chunks, budget \$$BUDGET, start parallel $P"
i=0
while [ "$i" -lt "$TOTAL" ]; do
  C=$(cost)
  if awk -v c="$C" -v b="$BUDGET" 'BEGIN{exit !(c>=b)}'; then echo "BUDGET \$$BUDGET reached (spent \$$C): stopping at chunk $i"; break; fi
  tail -n +$((i+1)) "$LIST" | head -n 400 > "$D/batch.tmp"
  FAILS=$( (xargs -P "$P" -L 1 bash -c "$HERE/mine_chunk.sh tier$TIER \$0 \$1 \$2 $OUT" < "$D/batch.tmp") 2>&1 >/dev/null | grep -c '^FAIL\|^BADJSON')
  i=$((i+400)); DONE=$(wc -l < "$OUT" 2>/dev/null || echo 0)
  if [ "$FAILS" -gt 40 ]; then P=$(( P/2 > 4 ? P/2 : 4 )); sleep 30; elif [ "$FAILS" -eq 0 ] && [ "$P" -lt 32 ]; then P=$((P+4)); fi
  echo "$(date +%H:%M) chunk $i/$TOTAL mined $DONE spent \$$(printf %.2f "$(cost)") fails $FAILS parallel $P"
done
touch "$D/DONE-tier$TIER"
echo "MINE-ALL-DONE tier $TIER"
