#!/bin/bash
# run2.sh <sampletsv> <outdir> [P]   Build (mkunit2) and prove (prove_unit2) every row of a sample; resumable (a unit with result.tsv is skipped). STOP file in <outdir> stops it.
HERE=$(cd "$(dirname "$0")" && pwd); S=$1; O=$2; P=${3:-12}
export SAMPLE=$S; mkdir -p "$O"
n=$(wc -l <"$S")
seq 1 "$n" | xargs -P "$P" -I{} bash -c '[ -f "'"$O"'/STOP" ] && exit 0; u="'"$O"'/u$(printf %03d {})"; [ -f "$u/result.tsv" ] && exit 0; bash "'"$HERE"'/mkunit2.sh" {} "'"$O"'" && bash "'"$HERE"'/prove_unit2.sh" "$u"'
