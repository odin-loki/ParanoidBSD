#!/bin/bash
# run_pilot.sh [P_PROVE] [P_ANNOT]   S4 pilot: build the units of sample.tsv, run Frama-C WP on each (phase 1), then GLM loop-annotation attempts on the FAILED ones (phase 2). Resumable: a unit with
# result.tsv (phase 1) / annot.tsv (phase 2) is skipped. Stop cleanly: create $D/STOP (also written by morph_call.sh on a credit error or when the USD cap is reached).
# Data: /mnt/c/ParanoidBSD/mined-data/pilot (persistent, not /tmp). Log: $D/run.log
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; U=$D/units
PP=${1:-8}; PA=${2:-4}; LOG=$D/run.log
say() { echo "$(date '+%F %T') $*" >>"$LOG"; }
n=$(wc -l <"$D/sample.tsv")
say "pilot run start: $n units, P_prove=$PP P_annot=$PA"
# toolchain manifest (pinned versions, hashes of the inputs)
{ . "$HERE/env.sh"; echo "frama-c $(frama-c -version)"; echo "why3 $(why3 --version)"; echo "alt-ergo $(alt-ergo --version)"; echo "z3 $(z3 --version)"; echo "cvc5 $(cvc5 --version | head -1)"
  echo "sample.tsv sha256 $(sha256sum <"$D/sample.tsv" | cut -c1-64)"; echo "prelude.h sha256 $(sha256sum <"$HERE/prelude.h" | cut -c1-64)"
  [ -f "$D/macros.uniq.tsv" ] && echo "macros.uniq.tsv sha256 $(sha256sum <"$D/macros.uniq.tsv" | cut -c1-64)"
  echo "scripts git $(git -C "$HERE" rev-parse HEAD 2>/dev/null)"; } >"$D/toolchain.txt" 2>&1
# units
for i in $(seq 1 "$n"); do [ -f "$U/u$(printf %03d "$i")/meta.tsv" ] || bash "$HERE/mkunit.sh" "$i" "$U"; done
# phase 1
ls -d "$U"/u* | while read -r d; do [ -f "$d/result.tsv" ] || echo "$d"; done >"$D/todo.prove"
say "phase 1: $(wc -l <"$D/todo.prove") units to prove"
xargs -a "$D/todo.prove" -P "$PP" -I{} bash -c '[ -f "$0/../STOP" ] || [ -f "'"$D"'/STOP" ] || bash "'"$HERE"'/prove_unit.sh" "$0"' {}
say "phase 1 done: $(cat "$U"/u*/result.tsv | cut -f5 | sort | uniq -c | tr '\n' ' ')"
# phase 2: only units whose contract WP could not prove and that have a loop or a goal left (status FAILED)
for d in "$U"/u*; do [ -f "$d/annot.tsv" ] && continue; [ "$(cut -f5 "$d/result.tsv" 2>/dev/null)" = FAILED ] && echo "$d"; done >"$D/todo.annot"
say "phase 2: $(wc -l <"$D/todo.annot") units to annotate"
[ -f "$D/STOP" ] || xargs -a "$D/todo.annot" -P "$PA" -I{} bash -c '[ -f "'"$D"'/STOP" ] || bash "'"$HERE"'/annotate_unit.sh" "$0"' {}
say "phase 2 done; spent $(awk -F'\t' '{s+=$8} END{printf "%.4f", s}' "$D/morph_ledger.tsv" 2>/dev/null) USD (conservative price)"
[ -f "$D/STOP" ] && say "STOP file present: $(cat "$D/STOP")" || touch "$D/DONE"
