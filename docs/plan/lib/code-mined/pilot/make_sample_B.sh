#!/bin/bash
# make_sample_B.sh   Extension sample B for the pilot: functions WITH A LOOP (the main-run cost driver for the model: loop invariants), same candidate pool and builder as sample A.
# 1. take 600 random scalar-contract candidates of the loop-prone kinds that are not in sample A, 2. extract each function (mkunit.sh), 3. keep the first 60 whose text contains for/while/do.
# Output: $B/sample.tsv (same columns as sample A). Run it, then:  PILOT_DATA=$B SAMPLE=$B/sample.tsv LEDGER=<main ledger> bash run_pilot.sh 8 14
D=${D:-/mnt/c/ParanoidBSD/mined-data/pilot}; B=${B:-/mnt/c/ParanoidBSD/mined-data/pilot-B}; HERE=$(cd "$(dirname "$0")" && pwd); mkdir -p "$B/tmp"
awk -F'\t' 'NR==FNR { s[$1 SUBSEP $5]=1; next } $9=="scalar" && ($7=="parser"||$7=="bitmap"||$7=="ring-buffer"||$7=="list-op"||$7=="arith-helper"||$7=="init-teardown"||$7=="allocator"||$7=="refcount"||$7=="bitmask") && !(($1 SUBSEP $5) in s)' "$D/sample.tsv" "$D/cands.cls.tsv" | shuf --random-source=<(yes pbsd-s4-pilot-B) | head -600 >"$B/tmp/pool.tsv"
n=$(wc -l <"$B/tmp/pool.tsv")
seq 1 "$n" | xargs -P 8 -I{} env SAMPLE="$B/tmp/pool.tsv" bash "$HERE/mkunit.sh" {} "$B/tmp/units"
: >"$B/sample.tsv"; kept=0
for i in $(seq 1 "$n"); do
  u="$B/tmp/units/u$(printf %03d "$i")"
  [ "$(cut -f10 "$u/meta.tsv" 2>/dev/null)" = ok ] || continue
  awk -f "$HERE/has_loop.awk" "$u/fn.c" || continue
  grep -qE '__asm__|template[ ]*<|::' "$u/fn.c" && continue
  sed -n "${i}p" "$B/tmp/pool.tsv" >>"$B/sample.tsv"; kept=$((kept+1)); [ $kept -ge 60 ] && break
done
echo "sample B: $kept loop-bearing functions from a pool of $n"; cut -f7 "$B/sample.tsv" | sort | uniq -c
