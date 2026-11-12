#!/bin/bash
# run2.sh <sampletsv> <outdir> [P]   Per row: build the unit (mkunit2), prove with WP (prove_unit2), and for units WP could not close (FAILED, CONTRACT-ONLY) run the
# bit-vector path (bv_unit). Resumable: a unit with result.tsv (and result.bv.tsv where due) is skipped. STOP file in <outdir> stops it.
HERE=$(cd "$(dirname "$0")" && pwd); S=$1; O=$2; P=${3:-12}; A=${4:-1}; Z=${5:-0}
export SAMPLE=$S; mkdir -p "$O"
n=$(wc -l <"$S"); [ "$Z" -gt 0 ] && [ "$Z" -lt "$n" ] && n=$Z
seq "$A" "$n" | xargs -P "$P" -I{} bash -c '
  [ -f "'"$O"'/STOP" ] && exit 0
  u="'"$O"'/u$(printf %03d {})"
  [ -f "$u/result.tsv" ] || { bash "'"$HERE"'/mkunit2.sh" {} "'"$O"'"; bash "'"$HERE"'/prove_unit2.sh" "$u"; }
  st=$(cut -f5 "$u/result.tsv" 2>/dev/null)
  if { [ "$st" = FAILED ] || [ "$st" = CONTRACT-ONLY ]; } && [ ! -f "$u/result.bv.tsv" ]; then bash "'"$HERE"'/bv_unit.sh" "$u"; fi
  exit 0'
