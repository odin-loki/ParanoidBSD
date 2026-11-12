#!/bin/bash
# prove_unit2.sh <unitdir>   Prove the mined contract of one unit (built by mkunit2.sh from the real translation unit) with Frama-C WP.
# Same checks as prove_unit.sh: A = contract goals, B = + run-time-error guards (-wp-rte), vacuity (assert \false at entry must NOT be provable). Annotation errors are fatal.
# Extra for step 2: WP_MODE=int (default: WP Typed model, provers alt-ergo,z3,cvc5) or WP_MODE=bv (Why3 bit-vector theory through WP's -wp-model Typed + provers z3,cvc5 with bitvector driver; see bv.md).
# result.tsv: N name kind class status tree nA_proved A_total B_proved B_total vacuous seconds cause assumptions(;-separated)
HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/env.sh"
U=$1; TO=${WP_TIMEOUT:-10}; PROVERS=${WP_PROVERS:-alt-ergo,z3,cvc5}
cd "$U" || exit 2; RES=${RESULT:-result.tsv}; UC=${UNITFILE:-unit.c}
IFS=$'\t' read -r N name kind cls file tree mstat detail <meta.tsv
t0=$(date +%s.%N); Ap=; At=; Bp=; Bt=; vac=
asum=""
emit() { # emit status cause
  secs=$(awk -v a="$t0" -v b="$(date +%s.%N)" 'BEGIN{printf "%.1f", b-a}')
  printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$kind" "$cls" "$1" "$tree" "${Ap:--}" "${At:--}" "${Bp:--}" "${Bt:--}" "${vac:--}" "$secs" "$2" "$asum" >"$RES"
  exit 0
}
[ "$mstat" = ok ] || emit UNBUILT "$detail"
# ---- per-unit assumptions (named; the label PROVED-ASSUMING lists them)
fnbody=$(awk -v n="$name" '{ if (!f && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && $0 !~ /;[ \t]*$/) f=1; if (f) print }' "$UC" | tail -n +1)
ca=""
if [ -s "$UC.callees" ]; then
  for cn in $(cat "$UC.callees"); do
    [ "$cn" = "$name" ] && continue
    if printf '%s' "$fnbody" | tr -c 'A-Za-z0-9_\n' ' ' | tr ' ' '\n' | grep -qx "$cn"; then ca="$ca$cn,"; fi
  done
fi
[ -n "$ca" ] && asum="${asum}callees-without-spec:${ca%,};"
sed -E 's#^[[:space:]]*/\*@##; s#\*/[[:space:]]*$##' contract.exp | grep -q -E '\\valid|\\separated|->|\*|\[' && asum="${asum}pointer-or-memory-precondition;"
case "$tree:$file" in linux:*/arch/x86/*|linux:*/arch/x86_64/*) ;; linux:*/arch/*) asum="${asum}x86_64-config-used-for-non-x86-arch-file;" ;; esac
grep -q 'probe=1' tu.how 2>/dev/null && asum="${asum}header-function-in-probe-unit;"
grep -q 'extra=+' tu.how 2>/dev/null && asum="${asum}probe-adds-module.h-kernel.h-slab.h-device.h;"
[ -s bsd.stubs ] && asum="${asum}stub-headers:$(tr '\n' ',' <bsd.stubs | sed 's/,$//');"
FC="frama-c -kernel-warn-key annot-error=active -machdep gcc_x86_64 -no-autoload-plugins"
fcrun() { f=$1; shift; OUT=$(timeout 600 frama-c -kernel-warn-key annot-error=active -machdep gcc_x86_64 -wp-timeout "${TOUT:-$TO}" -wp-prover "$PROVERS" -wp-fct "$name" "$@" "$f" 2>&1); RC=$?; }
OUT=$(timeout 120 $FC "$UC" 2>&1); RC=$?
if [ $RC -ne 0 ] || echo "$OUT" | grep -q -E 'annot-error|User Error'; then
  cause=""
  for pat in "annot-error" "User Error" "syntax error" "Cannot find field" "Cannot resolve variable" "unbound logic" "rror"; do
    cause=$(echo "$OUT" | grep -v -E '^\[kernel\] (Parsing|Warning: end)' | grep -m1 -A1 -E "$pat" | tr '\t\n' '  ' | sed -E 's/ +/ /g; s/ +$//' | cut -c1-160)
    [ -n "$cause" ] && break
  done
  echo "$OUT" >parse.log
  emit UNBUILT "$cause"
fi
cp "$UC" unit.final.c
fcrun unit.final.c -wp -wp-no-rte; echo "$OUT" >wp.A.log
Ap=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); At=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
[ -z "$At" ] && emit WPERROR "$(echo "$OUT" | grep -m1 -E 'rror|Aborted|aborted' | cut -c1-100)"
if [ "$Ap" != "$At" ]; then
  why="contract not proved"; awk -f "$HERE/has_loop.awk" fn.c 2>/dev/null && why="$why (loop)"
  echo "$OUT" | grep -q -i 'timeout' && why="$why; solver timeout"
  emit FAILED "$why"
fi
fcrun unit.final.c -wp -wp-rte; echo "$OUT" >wp.B.log
Bp=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); Bt=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
# vacuity
awk -v n="$name" 'BEGIN{d=0} { if (!d && f_in==0 && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && $0 !~ /;[ \t]*$/) f_in=1; if (f_in && !d && (i = index($0, "{")) > 0) { $0 = substr($0, 1, i) " /*@ assert \\false; */" substr($0, i+1); d=1 } print }' "$UC" >unit.vac.c
TOUT=${VAC_TIMEOUT:-5} fcrun unit.vac.c -wp -wp-no-rte
Vp=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1/p' | tail -1); Vt=$(echo "$OUT" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\2/p' | tail -1)
vac=no; [ -n "$Vt" ] && [ "$Vp" = "$Vt" ] && vac=YES
[ "$vac" = YES ] && emit VACUOUS "requires is contradictory"
if [ "$Bp" = "$Bt" ] && [ -n "$Bt" ]; then emit PROVED ""; fi
emit CONTRACT-ONLY "contract proved; run-time-error guards not all proved ($Bp/$Bt)"
