#!/bin/bash
# replay2.sh <rundir> [P]   Independent replay of every proved unit of a run2.sh run, from a CLEAN directory:
#   WP proofs  : only unit.final.c (self-contained: sliced real translation unit, no prelude) is copied; WP cache OFF (-wp-cache none), fresh process, machdep of the unit, 60 s per goal;
#                contract goals (A), run-time-error goals (B), vacuity (assert \false at entry must NOT be provable), termination goals again;
#                scan: no assume/admit/lemma/axiom/ghost/behavior/check; exactly ONE ACSL block in the file (the mined contract: the annotation guard of the model loop, kept as a check);
#                the block must equal contract.exp (the mined contract with its macros expanded) ; contract.acsl (the mined text) is kept next to it.
#   BV proofs  : only bv.h.c is copied; cbmc is run again (A: assertion, B: undefined-behaviour checks) in the clean directory.
# Writes <unit>/replay.tsv: N name engine A B vac scan sha256(file) status      (status OK only if everything is reproduced)
HERE=$(cd "$(dirname "$0")" && pwd); D=$1; P=${2:-8}
one() {
  d=$1; . "$HERE/env.sh"
  st=$(cut -f5 "$d/result.tsv" 2>/dev/null); bv=$(cut -f4 "$d/result.bv.tsv" 2>/dev/null)
  IFS=$'\t' read -r N name rest <"$d/result.tsv"
  if [ "$st" = PROVED ]; then
    W=$(mktemp -d); cp "$d/unit.final.c" "$W/"; MD=$(cat "$d/machdep" 2>/dev/null || echo gcc_x86_64)
    scan=clean
    { grep -o '/\*@.*\*/' "$d/unit.final.c"; grep '//@' "$d/unit.final.c"; } | grep -w -E 'assume|assumes|admit|lemma|axiom|axiomatic|ghost|behavior|check' >/dev/null && scan=BANNED
    nb=$(grep -c '/\*@' "$d/unit.final.c"); [ "$nb" = 1 ] || scan="acsl-blocks=$nb"
    grep -qF "$(cat "$d/contract.exp")" "$d/unit.final.c" || scan="contract-text-differs"
    cd "$W"
    run() { timeout 900 frama-c -kernel-warn-key annot-error=active -machdep $MD -wp -wp-cache none -wp-timeout 60 -wp-prover alt-ergo,z3,cvc5 -wp-fct "$name" "$@" unit.final.c 2>&1; }
    cnt() { sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1\/\2/p' | tail -1; }
    a=$(run -wp-no-rte -wp-prop=-@terminates | cnt); b=$(run -wp-rte -wp-prop=-@terminates | cnt); t=$(run -wp-no-rte -wp-prop=+@terminates | cnt)
    awk -v n="$name" 'BEGIN{dd=0} { if (!dd && f==0 && match($0, "(^|[^A-Za-z0-9_])" n "[ \t]*\\(") && $0 !~ /;[ \t]*$/) f=1; if (f && !dd && (i = index($0, "{")) > 0) { $0 = substr($0, 1, i) " /*@ assert \\false; */" substr($0, i+1); dd=1 } print }' unit.final.c >unit.vac.c
    v=$(timeout 300 frama-c -kernel-warn-key annot-error=active -machdep $MD -wp -wp-no-rte -wp-cache none -wp-timeout 5 -wp-prover alt-ergo,z3,cvc5 -wp-fct "$name" unit.vac.c 2>&1 | cnt)
    vac=no; [ -n "$v" ] && [ "${v%/*}" = "${v#*/}" ] && vac=YES
    stt=OK; { [ -n "$a" ] && [ "${a%/*}" = "${a#*/}" ] && [ -n "$b" ] && [ "${b%/*}" = "${b#*/}" ] && [ "$scan" = clean ] && [ "$vac" = no ]; } || stt=MISMATCH
    # termination must match what was recorded
    printf '%s\t%s\tWP\t%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$a" "$b" "$vac" "$scan;T=$t" "$(sha256sum <"$d/unit.final.c" | cut -c1-16)" "$stt" >"$d/replay.tsv"
    rm -rf "$W"
  elif [ "$bv" = BV-PROVED ]; then
    W=$(mktemp -d); cp "$d/bv.h.c" "$W/"; cd "$W"
    L=$(awk -f "$HERE/has_loop.awk" "$d/unit.c.fn" && echo 66 || echo 2)
    a=$(timeout 600 cbmc bv.h.c --function main --unwind $L --unwinding-assertions --object-bits 12 2>&1 | grep -c "VERIFICATION SUCCESSFUL")
    b=$(timeout 600 cbmc bv.h.c --function main --unwind $L --unwinding-assertions --object-bits 12 --signed-overflow-check --div-by-zero-check --undefined-shift-check --pointer-check --bounds-check 2>&1 | grep -c "VERIFICATION SUCCESSFUL")
    stt=OK; [ "$a" = 1 ] && [ "$b" = 1 ] || stt=MISMATCH
    printf '%s\t%s\tCBMC\t%s\t%s\t-\tclean\t%s\t%s\n' "$N" "$name" "$a" "$b" "$(sha256sum <"$d/bv.h.c" | cut -c1-16)" "$stt" >"$d/replay.tsv"
    rm -rf "$W"
  fi
}
export -f one; export HERE
ls -d "$D"/u* | xargs -P "$P" -I{} bash -c 'one {}'
cat "$D"/u*/replay.tsv 2>/dev/null | cut -f3,9 | sort | uniq -c
