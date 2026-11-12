#!/bin/bash
# bv_unit.sh <unitdir>   Step 2, bit-vector path: decide the mined SCALAR contract with CBMC (bit-precise SAT) on the unit that WP could not prove.
# Inputs (from mkunit2.sh): unit.c (sliced real translation unit + contract), unit.c.sig, contract.exp.   Output: result.bv.tsv and bv.*.log
#   status  BV-PROVED       contract holds for ALL inputs meeting `requires` (loop-free code, or unwinding assertions hold), no undefined behaviour found (CBMC checks:
#                           signed overflow, division by zero, undefined shift, bounds, pointer), `requires` satisfiable (vacuity check)
#           BV-CONTRACT-ONLY  contract holds, but an input that meets `requires` causes undefined behaviour (the contract is too weak to exclude it)
#           BV-FALSE        counterexample: an input meeting `requires` that violates `ensures` under C semantics  -> goes to the repair list
#           BV-VACUOUS      `requires` is unsatisfiable
#           BV-UNWIND       loop not fully unwound: only BOUNDED
#           BV-SKIP         contract or signature not in the supported scalar subset (reason given), BV-NOPARSE CBMC cannot read the unit, BV-TIMEOUT
# Soundness notes (see acsl2bv.cpp): ACSL math integers are evaluated in __int128 with overflow/shift/division assertions that FAIL rather than being assumed away.
# Trust: CBMC and its SAT solver are not proof-certified, so the label is PROVED-ASSUMING (bv-sat-uncertified), never PROVED.
HERE=$(cd "$(dirname "$0")" && pwd)
U=$1; cd "$U" || exit 2
IFS=$'\t' read -r N name kind cls file tree mstat detail <meta.tsv
T=${BV_TIMEOUT:-120}; t0=$(date +%s.%N)
emit() { secs=$(awk -v a="$t0" -v b="$(date +%s.%N)" 'BEGIN{printf "%.1f", b-a}'); printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$kind" "$1" "$secs" "$2" >result.bv.tsv; exit 0; }
[ "$mstat" = ok ] && [ -s unit.c.sig ] || emit BV-SKIP "no built unit"
r=$("${ACSL2BV:-$HOME/pbsd-tools/acsl2bv}" contract.exp unit.c.sig unit.c "$name" bv.h.c); [ $? -eq 0 ] || emit BV-SKIP "${r#SKIP }"
# function text only, for the loop test
awk -v n="$name" '{ if (!f && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && $0 !~ /;[ \t]*$/) f=1; if (f) print }' unit.c >unit.c.fn
if grep -q -E '(^|[^A-Za-z0-9_])(asm|__asm__|__asm)([^A-Za-z0-9_]|$)|__builtin_ia32' unit.c.fn; then emit BV-SKIP "inline asm / target builtin in the function: CBMC has no semantics for it (neither proof nor counterexample is trusted)"; fi
awk -f "$HERE/has_loop.awk" unit.c.fn && U_=${BV_UNWIND:-66} || U_=2
base="cbmc bv.h.c --function main --unwind $U_ --unwinding-assertions --object-bits 12"
# A: ensures only
timeout $T $base >bv.A.log 2>&1; rcA=$?
if grep -q "CONVERSION ERROR\|PARSING ERROR\|Type checking failed\|error: " bv.A.log && ! grep -q "VERIFICATION" bv.A.log; then emit BV-NOPARSE "$(grep -m1 -E 'error|ERROR' bv.A.log | cut -c1-140)"; fi
[ $rcA -eq 124 ] && emit BV-TIMEOUT "A"
fails=$(grep -E ": FAILURE$" bv.A.log | sed -E 's/^\[([^]]*)\].*/\1/')
if [ -n "$fails" ]; then
  if echo "$fails" | grep -q "unwind"; then emit BV-UNWIND "loop not fully unwound at $U_"; fi
  if echo "$fails" | grep -q "main.assertion\|assertion"; then
    # counterexample inputs
    cex=$(timeout 60 cbmc bv.h.c --function main --unwind $U_ --object-bits 12 --trace 2>/dev/null | grep -E "^  [A-Za-z_][A-Za-z0-9_]*=" | head -8 | tr '\n' ' ')
    echo "$cex" >bv.cex
    # CBMC gives inline asm / unknown builtins no semantics (outputs become arbitrary): a counterexample is then not evidence against the contract (a PROOF stays sound)
    if grep -q -E '(^|[^A-Za-z0-9_])(asm|__asm__|__asm)([^A-Za-z0-9_]|$)|__builtin_ia32' unit.c.fn; then emit BV-INCONCLUSIVE-ASM "counterexample may come from inline asm treated as arbitrary: $cex"; fi
    emit BV-FALSE "violates ensures under C semantics: $cex"
  fi
fi
grep -q "VERIFICATION SUCCESSFUL" bv.A.log || emit BV-NOPARSE "no verdict: $(tail -1 bv.A.log | cut -c1-100)"
# vacuity: is the assertion after the assume reachable?
sed 's|__CPROVER_assert(\(.*\), "ensures");|__CPROVER_assert(0, "reach");|' bv.h.c >bv.vac.c
timeout $((T+30)) cbmc bv.vac.c --function main --unwind $U_ --object-bits 12 >bv.V.log 2>&1
grep -q "VERIFICATION SUCCESSFUL" bv.V.log && emit BV-VACUOUS "requires is unsatisfiable"
# B: undefined behaviour of the function itself
timeout $T $base --signed-overflow-check --div-by-zero-check --undefined-shift-check --pointer-check --bounds-check >bv.B.log 2>&1; rcB=$?
[ ${rcB:-0} -eq 124 ] && emit BV-TIMEOUT "B"
ub=$(grep -E ": FAILURE$" bv.B.log | sed -E 's/^\[([^]]*)\].*/\1/' | grep -v -E '^(bv_|main\.|bv_chk)' | head -3 | tr '\n' ' ')
[ -n "$ub" ] && emit BV-CONTRACT-ONLY "undefined behaviour possible: $ub"
grep -q "VERIFICATION SUCCESSFUL" bv.B.log || emit BV-CONTRACT-ONLY "UB check inconclusive: $(tail -1 bv.B.log | cut -c1-80)"
emit BV-PROVED ""
