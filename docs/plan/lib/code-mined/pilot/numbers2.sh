#!/bin/bash
# numbers2.sh   Tables for the step 1/2 report, regenerated from data2/{A,C}/labels.tsv (and data/ of the pilot for the "before" column of sample A).
HERE=$(cd "$(dirname "$0")" && pwd); D=$HERE/data2
echo "### Proved by kind: P = PROVED, PA = PROVED-ASSUMING   (pilot = before; s1 = after real headers, WP only; s1+2 = plus bit-vector path)"
echo
echo "| kind | n (A / C) | pilot A: P+PA | A s1: P+PA | A s1+2: P+PA | C s1: P+PA | C s1+2: P+PA |"
echo "|---|---|---|---|---|---|---|"
for k in arith-helper init-teardown error-path parser lock-or-sync allocator bitmap ring-buffer refcount list-op bitmask; do
  pa=$(awk -F'\t' -v k=$k '$3==k && $5 ~ /^PROVED/' "$HERE/data/results.tsv" 2>/dev/null | wc -l)
  r() { f=$1; k=$2; w=$3; awk -F'\t' -v k=$k -v w=$w '$3==k && ($6=="PROVED"||$6=="PROVED-ASSUMING") && (w=="all" || $7=="WP"){n++; if($6=="PROVED") p++} END{printf "%d+%d", p, n-p}' "$f"; }
  nA=$(awk -F'\t' -v k=$k '$3==k' "$D/A/labels.tsv" | wc -l); nC=$(awk -F'\t' -v k=$k '$3==k' "$D/C/labels.tsv" | wc -l)
  echo "| $k | $nA / $nC | $pa (total) | $(r $D/A/labels.tsv $k wp) | $(r $D/A/labels.tsv $k all) | $(r $D/C/labels.tsv $k wp) | $(r $D/C/labels.tsv $k all) |"
done
for s in A C; do
  echo; echo "### Sample $s totals"
  awk -F'\t' '{n[$6]++} END{for(l in n) printf "- %s: %d\n", l, n[l]}' "$D/$s/labels.tsv" | sort
  echo "- bit-vector proofs (CBMC): $(awk -F'\t' '$7=="CBMC-bitvector" && $6=="PROVED-ASSUMING"' "$D/$s/labels.tsv" | wc -l);  of the WP-proved: PROVED $(awk -F'\t' '$7=="WP" && $6=="PROVED"' "$D/$s/labels.tsv" | wc -l), PROVED-ASSUMING $(awk -F'\t' '$7=="WP" && $6=="PROVED-ASSUMING"' "$D/$s/labels.tsv" | wc -l)"
  echo "- by class: $(awk -F'\t' '$4=="scalar"{n++; if($6 ~ /^PROVED/) p++} END{printf "scalar %d/%d", p, n}' "$D/$s/labels.tsv"), $(awk -F'\t' '$4!="scalar"{n++; if($6 ~ /^PROVED/) p++} END{printf "pointer/field %d/%d", p, n}' "$D/$s/labels.tsv")"
  echo "- unbuilt by tree: $(awk -F'\t' '$6=="UNBUILT"{n[$5]++} END{for(t in n) printf "%s %d; ", t, n[t]}' "$D/$s/labels.tsv")"
done
echo; echo "### Replay from a clean directory (cache off)"
cat "$D"/*/units/*/replay.tsv | cut -f3,9 | sort | uniq -c | sed 's/^/- /'
