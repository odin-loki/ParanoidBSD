#!/bin/bash
# spend.sh repair|review <ledger>   USD upper bound for one ledger (price: GLM 0.047/0.268, DeepSeek 0.12/0.80 per 1M tokens).
# spend.sh all                      Whole job (rounds 2+): every repair<N>/ledger.txt and verify-repair<N>/xcheck/ledger.txt, plus round 1's review ledger above its baseline
#                                   (final/spend-baseline.txt = tokens already spent when the job started).
D=/mnt/c/ParanoidBSD/mined-data
price() { awk -v pi="$1" -v po="$2" '{i+=$1; o+=$2} END{printf "%.4f", (i*pi+o*po)/1000000}' "$3" 2>/dev/null || echo 0; }
case "$1" in
  repair) price 0.047 0.268 "$2";;
  review) price 0.12 0.80 "$2";;
  all) t=0
    for f in "$D"/repair[0-9]*/ledger.txt; do [ -f "$f" ] && t=$(awk -v a="$t" -v b="$(price 0.047 0.268 "$f")" 'BEGIN{printf "%.4f", a+b}'); done
    for f in "$D"/verify-repair[0-9]*/xcheck/ledger.txt; do [ -f "$f" ] && t=$(awk -v a="$t" -v b="$(price 0.12 0.80 "$f")" 'BEGIN{printf "%.4f", a+b}'); done
    base=$(cat "$D/final/spend-baseline.txt" 2>/dev/null || echo 0); cur=$(price 0.12 0.80 "$D/verify-repair/xcheck/ledger.txt")
    awk -v a="$t" -v c="$cur" -v b="$base" 'BEGIN{printf "%.2f\n", a + (c > b ? c - b : 0)}';;
esac
