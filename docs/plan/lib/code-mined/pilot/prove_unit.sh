#!/bin/bash
# prove_unit.sh <unitdir>   Try to prove the mined contract of one unit with Frama-C WP (+ Alt-Ergo, Z3, CVC5 through Why3). Writes <unitdir>/result.tsv (one line) and logs.
#
# Steps
#   1. Build the unit (unit.c = prelude + file #defines + resolved.h + file types + contract + function; fallback unit.min.c without the types).
#      Names the compiler cannot find (Cannot resolve variable X / unbound logic variable X) are looked up in the macro index (macros.uniq.tsv: names with ONE definition in all headers)
#      and added to resolved.h; repeated until the unit parses or nothing new can be resolved. The resolved names are listed in the result (they are an assumption: PROVED-ASSUMING).
#   2. WP A: the contract goals only (ensures/assigns/terminates), then WP B: the same with -wp-rte (no undefined behaviour: overflow of signed ints, shifts, division, array bounds, pointers).
#   3. Vacuity check: the same unit with `assert \false;` at function entry. If WP proves it, the precondition is contradictory and the proof is worthless -> VACUOUS.
#   PROVED needs A and B fully proved and not vacuous. Annotation errors are fatal (-kernel-warn-key annot-error=active), so a contract that Frama-C ignored can never give a proof.
# result.tsv: N name kind class status level nresolved A_proved A_total B_proved B_total vacuous seconds cause resolved-names
HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/env.sh"
U=$1; IDX=${MACRO_INDEX:-/mnt/c/ParanoidBSD/mined-data/pilot/macros.uniq.tsv}
TO=${WP_TIMEOUT:-10}; PROVERS=${WP_PROVERS:-alt-ergo,z3,cvc5}
cd "$U" || exit 2; FN=${FNFILE:-fn.c}; RES=${RESULT:-result.tsv}
IFS=$'\t' read -r N name kind cls file lines hc hf hp mstat <"meta.tsv"
t0=$(date +%s.%N)
emit() { # emit status level cause
  secs=$(awk -v a="$t0" -v b="$(date +%s.%N)" 'BEGIN{printf "%.1f", b-a}')
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$kind" "$cls" "$1" "$2" "$nres" "${Ap:--}" "${At:--}" "${Bp:--}" "${Bt:--}" "${vac:--}" "$secs" "$3" "$(tr '\n' ',' <resolved.names 2>/dev/null)" >"$RES"
  exit 0
}
nres=0; : >resolved.names; : >resolved.h; Ap=; At=; Bp=; Bt=; vac=
[ "$mstat" = ok ] || emit UNBUILT none "function definition not found in the file"
assemble() { # assemble <withtypes 0|1>  -> unit.cur.c
  { echo '#include "prelude.h"'; cat defs.h resolved.h; [ "$1" = 1 ] && cat types.h; cat contract.acsl "$FN"; } >unit.cur.c
}
fcrun() { # fcrun <file> <extra args...>   -> $OUT
  f=$1; shift
  OUT=$(timeout 600 frama-c -kernel-warn-key annot-error=active -machdep x86_64 -wp-timeout "${TOUT:-$TO}" -wp-prover "$PROVERS" -wp-fct "$name" "$@" "$f" 2>&1); RC=$?
}
parse_ok() { # parse only, same flags
  OUT=$(timeout 120 frama-c -kernel-warn-key annot-error=active -machdep x86_64 unit.cur.c 2>&1); RC=$?
  [ $RC -eq 0 ] && ! echo "$OUT" | grep -q -E 'annot-error|User Error'
}
level=none
for wt in 1 0; do
  : >resolved.h; : >resolved.names; nres=0
  for round in 1 2 3 4 5 6 7 8; do
    assemble $wt
    if parse_ok; then level=$([ $wt = 1 ] && echo L2 || echo L1); break 2; fi
    ids=$(echo "$OUT" | grep -oE '(Cannot resolve variable|unbound logic variable|undeclared identifier|Unbound logic variable) [A-Za-z_][A-Za-z0-9_]*' | awk '{print $NF}' | sort -u)
    new=0
    for id in $ids; do
      grep -qx "$id" resolved.names && continue
      line=$(LC_ALL=C grep -m1 -- "^$id"$'\t' "$IDX" | cut -f2)
      [ -z "$line" ] && continue
      { echo "#undef $id"; echo "$line"; } >>resolved.h; echo "$id" >>resolved.names; new=$((new+1)); nres=$((nres+1))
    done
    [ $new -eq 0 ] && break
  done
done
if [ "$level" = none ]; then
  cause=$(echo "$OUT" | grep -v -E '^\[kernel\] (Parsing|Warning: end)' | grep -m1 -A1 -E "Cannot resolve variable|Cannot find field|syntax error|annot-error|unbound logic|User Error|rror" | tr '\t\n' '  ' | sed -E 's/ +/ /g' | cut -c1-110)
  case "$cause" in
    *"Cannot resolve variable"*|*"unbound logic"*) c="unresolved symbol: ${cause##* }" ;;
    *"Cannot find field"*) c="missing struct type (field)" ;;
    *"syntax error"*) c="syntax error (macro or kernel syntax not parsed)" ;;
    *annot-error*) c="contract not valid ACSL: $(echo "$OUT" | grep -m1 -A1 'annot-error' | tr '\n' ' ' | sed -E 's/.*Warning: *//' | cut -c1-80)" ;;
    *) c="$cause" ;;
  esac
  emit UNBUILT none "$c"
fi
cp unit.cur.c unit.final.c
# A: contract goals
fcrun unit.final.c -wp -wp-no-rte
echo "$OUT" >wp.A.log
Ap=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); At=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
if [ -z "$At" ]; then emit WPERROR "$level" "$(echo "$OUT" | grep -m1 -E 'rror|Aborted|aborted' | cut -c1-100)"; fi
if [ "$Ap" != "$At" ]; then
  why="contract not proved"; grep -qE '\b(for|while|do)\b' "$FN" && why="contract not proved (function has a loop; needs loop invariants)"
  echo "$OUT" | grep -q -i 'timeout' && why="$why; solver timeout"
  emit FAILED "$level" "$why"
fi
# B: with run-time-error guards
fcrun unit.final.c -wp -wp-rte
echo "$OUT" >wp.B.log
Bp=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); Bt=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
# vacuity: assert \false at entry of the function body
awk 'BEGIN{d=0} { if (!d && (i = index($0, "{")) > 0) { $0 = substr($0, 1, i) " /*@ assert \\false; */" substr($0, i+1); d=1 } print }' "$FN" >fn.vac.c
{ echo '#include "prelude.h"'; cat defs.h resolved.h; [ "$level" = L2 ] && cat types.h; cat contract.acsl fn.vac.c; } >unit.vac.c
TOUT=${VAC_TIMEOUT:-5} fcrun unit.vac.c -wp -wp-no-rte
Vp=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); Vt=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
vac=no; [ -n "$Vt" ] && [ "$Vp" = "$Vt" ] && vac=YES
[ "$vac" = YES ] && emit VACUOUS "$level" "requires is contradictory"
if [ "$Bp" = "$Bt" ] && [ -n "$Bt" ]; then emit PROVED "$level" ""; fi
emit CONTRACT-ONLY "$level" "contract proved; run-time-error guards not all proved ($Bp/$Bt)"
