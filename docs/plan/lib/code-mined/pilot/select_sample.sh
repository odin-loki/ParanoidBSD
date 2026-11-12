#!/bin/bash
# select_sample.sh  Stratified, seeded sample of the S4 pilot from cands.tsv. Output sample.tsv (same columns as cands.tsv + class column 9: scalar|ptr).
# Quota per kind; within a kind ~75% functions whose contract mentions no pointer/field/array ("scalar"), the rest "ptr". Fills from the other class if one runs short.
D=${D:-/mnt/c/ParanoidBSD/mined-data/pilot}; SEED=${SEED:-pbsd-s4-pilot-1}
awk -f $(dirname $0)/cls.awk $D/cands.tsv > $D/cands.cls.tsv
: > $D/sample.tsv
for spec in arith-helper:50 init-teardown:20 error-path:20 parser:20 lock-or-sync:15 allocator:15 bitmap:15 ring-buffer:15 refcount:12 list-op:10 bitmask:8; do
  k=${spec%%:*}; q=${spec##*:}; qs=$(( q * 3 / 4 )); qp=$(( q - qs ))
  awk -F'\t' -v k=$k '$7==k && $9=="scalar"' $D/cands.cls.tsv | shuf --random-source=<(yes $SEED) > $D/.s.tmp
  awk -F'\t' -v k=$k '$7==k && $9=="ptr"' $D/cands.cls.tsv | shuf --random-source=<(yes $SEED) > $D/.p.tmp
  ns=$(wc -l < $D/.s.tmp); np=$(wc -l < $D/.p.tmp)
  [ $ns -lt $qs ] && { qp=$(( qp + qs - ns )); qs=$ns; }
  [ $np -lt $qp ] && { qs=$(( qs + qp - np )); qp=$np; }
  head -n $qs $D/.s.tmp >> $D/sample.tsv; head -n $qp $D/.p.tmp >> $D/sample.tsv
done
rm -f $D/.s.tmp $D/.p.tmp
wc -l $D/sample.tsv; cut -f7,9 $D/sample.tsv | sort | uniq -c
