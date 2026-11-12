#!/bin/bash
# bv_unit.sh <unitdir>   Bit-vector path: decide the mined contract with CBMC (bit-precise SAT) on the unit that WP could not prove. Scalar contracts (round 2) and, since round 3,
# contracts with pointer parameters whose objects are bounded by the contract.
# Inputs (from mkunit2.sh): unit.c (sliced real translation unit + contract), unit.c.sig, contract.exp.   Output: result.bv.tsv (N name kind status seconds detail) and bv.*.log
#   status  BV-PROVED         contract holds for ALL inputs meeting `requires` (loop-free code, or unwinding assertions hold at some bound up to 130), no undefined behaviour found
#                             (CBMC checks: signed overflow, division by zero, undefined shift, bounds, pointer), `requires` satisfiable (vacuity run)
#           BV-BOUNDED        as BV-PROVED but only for objects of at most 8 elements (a `\valid(p + (0..n-1))` with a non-literal n): a BOUNDED result, never PROVED
#           BV-CONTRACT-ONLY  contract holds (or only fails through undefined behaviour), but an input that meets `requires` causes undefined behaviour
#           BV-FALSE          counterexample: an input meeting `requires` that violates `ensures` under C semantics, without undefined behaviour on the way  -> repair list
#           BV-VACUOUS, BV-UNWIND (loop not unwound up to 130: BOUNDED at best), BV-SKIP (reason), BV-NOPARSE, BV-TIMEOUT, BV-INCONCLUSIVE-ASM
# detail of a BV-PROVED/BV-BOUNDED result = the assumptions of the model, ';'-separated (summarize2.sh copies them into the label):
#   bv-sat-uncertified; pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid); unwind=N; bound<=8
# Soundness notes (see acsl2bv.cpp): ACSL math integers are evaluated in __int128 with overflow/shift/division assertions that FAIL rather than being assumed away; a pointer parameter
# is a malloc object of exactly the size the `\valid` clause states (so an access past it is an out-of-bounds failure), or, without a `\valid`, NULL-or-one-object (a dereference then fails).
HERE=$(cd "$(dirname "$0")" && pwd)
U=$1; cd "$U" || exit 2
IFS=$'\t' read -r N name kind cls file tree mstat detail <meta.tsv
T=${BV_TIMEOUT:-120}; t0=$(date +%s.%N); RES=${BV_RESULT:-result.bv.tsv}
emit() { secs=$(awk -v a="$t0" -v b="$(date +%s.%N)" 'BEGIN{printf "%.1f", b-a}'); printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$kind" "$1" "$secs" "$2" >"$RES"; exit 0; }
[ "$mstat" = ok ] && [ -s unit.c.sig ] || emit BV-SKIP "no built unit"
r=$("${ACSL2BV:-$HOME/pbsd-tools/acsl2bv}" contract.exp unit.c.sig unit.c "$name" bv.h.c); [ $? -eq 0 ] || emit BV-SKIP "${r#SKIP }"
ptrs=no; bounded=no; case "$r" in *pointers*) ptrs=yes ;; esac; case "$r" in *bounded=8*) bounded=yes ;; esac
awk -v n="$name" '{ if (!f && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && $0 !~ /;[ \t]*$/) f=1; if (f) print }' unit.c >unit.c.fn
if grep -q -E '(^|[^A-Za-z0-9_])(asm|__asm__|__asm)([^A-Za-z0-9_]|$)|__builtin_ia32' unit.c.fn; then emit BV-SKIP "inline asm / target builtin in the function: CBMC has no semantics for it (neither proof nor counterexample is trusted)"; fi
awk -f "$HERE/has_loop.awk" unit.c.fn && loops=yes || loops=no
CH="--pointer-check --bounds-check --signed-overflow-check --div-by-zero-check --undefined-shift-check"
# unwinding bounds tried: loop-free -> 2; loops -> 10, 34, 66, 130 until the unwinding assertions hold
if [ $loops = yes ]; then BOUNDS="${BV_UNWIND:-10 34 66 130}"; else BOUNDS=2; fi
used=""
for U_ in $BOUNDS; do
  base="cbmc bv.h.c --function main --unwind $U_ --unwinding-assertions --object-bits 12"
  timeout $T $base >bv.A.log 2>&1; rcA=$?; used=$U_
  [ $rcA -eq 124 ] && emit BV-TIMEOUT "A at unwind $U_"
  if grep -q "CONVERSION ERROR\|PARSING ERROR\|Type checking failed\|error: " bv.A.log && ! grep -q "VERIFICATION" bv.A.log; then emit BV-NOPARSE "$(grep -m1 -E 'error|ERROR' bv.A.log | cut -c1-140)"; fi
  fails=$(grep -E ": FAILURE$" bv.A.log | sed -E 's/^\[([^]]*)\].*/\1/')
  if echo "$fails" | grep -q "unwind"; then
    # an unwinding failure may hide a real assertion failure behind it only if the assertion failure is also listed
    echo "$fails" | grep -q "assertion" || continue
  fi
  break
done
if [ -n "$fails" ]; then
  echo "$fails" | grep -q "unwind" && ! echo "$fails" | grep -q "assertion" && emit BV-UNWIND "loop not fully unwound at $used"
  if echo "$fails" | grep -q "assertion"; then
    cex=$(timeout 60 cbmc bv.h.c --function main --unwind $used --object-bits 12 --trace 2>/dev/null | grep -E "^  [A-Za-z_][A-Za-z0-9_]*=" | head -8 | tr '\n' ' ')
    echo "$cex" >bv.cex
    if [ $ptrs = yes ]; then   # a failing ensures through a wild/NULL dereference is undefined behaviour admitted by the contract, not a false contract
      timeout $T cbmc bv.h.c --function main --unwind $used --object-bits 12 $CH >bv.U.log 2>&1
      ubp=$(grep -E ": FAILURE$" bv.U.log | sed -E 's/^\[([^]]*)\].*/\1/' | grep -v -E '^(bv_|main\.assertion|main\.pointer_dereference.*bv_)' | grep -v "unwind" | head -3 | tr '\n' ' ')
      [ -n "$ubp" ] && emit BV-CONTRACT-ONLY "contract admits undefined behaviour (ensures also fails there): $ubp"
    fi
    emit BV-FALSE "violates ensures under C semantics: $cex"
  fi
fi
grep -q "VERIFICATION SUCCESSFUL" bv.A.log || emit BV-NOPARSE "no verdict: $(tail -1 bv.A.log | cut -c1-100)"
# vacuity: is the assertion after the assume reachable?
sed 's|__CPROVER_assert(\(.*\), "ensures");|__CPROVER_assert(0, "reach");|' bv.h.c >bv.vac.c
timeout $((T+30)) cbmc bv.vac.c --function main --unwind $used --object-bits 12 >bv.V.log 2>&1
grep -q "VERIFICATION SUCCESSFUL" bv.V.log && emit BV-VACUOUS "requires is unsatisfiable"
# B: undefined behaviour of the function itself
timeout $T $base $CH >bv.B.log 2>&1; rcB=$?
[ $rcB -eq 124 ] && emit BV-TIMEOUT "B"
ub=$(grep -E ": FAILURE$" bv.B.log | sed -E 's/^\[([^]]*)\].*/\1/' | grep -v -E '^(bv_|main\.|bv_chk)' | head -3 | tr '\n' ' ')
[ -n "$ub" ] && emit BV-CONTRACT-ONLY "undefined behaviour possible: $ub"
grep -q "VERIFICATION SUCCESSFUL" bv.B.log || emit BV-CONTRACT-ONLY "UB check inconclusive: $(tail -1 bv.B.log | cut -c1-80)"
as="bv-sat-uncertified;"
[ $ptrs = yes ] && as="${as}pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid);"
[ $loops = yes ] && as="${as}unwind=$used;"
if [ $bounded = yes ]; then emit BV-BOUNDED "${as}bound<=8-elements"; fi
emit BV-PROVED "$as"
