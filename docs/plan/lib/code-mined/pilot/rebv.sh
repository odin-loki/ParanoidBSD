#!/bin/bash
# rebv.sh <rundir> [P]   Rerun the bit-vector path (bv_unit.sh, round 3: pointers, loops) on every unit WP could not close, replacing result.bv.tsv; the old result is kept as result.bv1.tsv.
HERE=$(cd "$(dirname "$0")" && pwd); R=$1; P=${2:-12}
ls -d "$R"/u* | xargs -P "$P" -I{} bash -c '
  u={}; st=$(cut -f5 "$u/result.tsv" 2>/dev/null)
  { [ "$st" = FAILED ] || [ "$st" = CONTRACT-ONLY ]; } || exit 0
  [ -f "$u/result.bv2.tsv" ] && exit 0
  [ -f "$u/result.bv.tsv" ] && [ ! -f "$u/result.bv1.tsv" ] && cp "$u/result.bv.tsv" "$u/result.bv1.tsv"
  BV_RESULT=result.bv2.tsv bash "'"$HERE"'/bv_unit.sh" "$u"
  cp "$u/result.bv2.tsv" "$u/result.bv.tsv"'
