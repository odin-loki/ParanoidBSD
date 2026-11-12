#!/bin/bash
# mutate_proved.sh [P]   Mutation test (SELLABLE_PLAN S5, small version) of the PROVED units: break the C code in one place and require WP to FAIL on the unchanged contract.
# Mutants (first occurrence inside the function text only, never in the contract): eq (== -> !=), lt (< -> <=), gt (> -> >=), and (&& -> ||), add (+ -> -), band (& -> |), shl (<< -> >>), ret (return 0 -> return 1 / return 1 -> return 0).
# A mutant is KILLED if Frama-C WP no longer proves all contract goals (A run). Survivors are listed: they are either equivalent mutants (the change cannot be observed through the contract) or evidence that the contract is too weak. Writes <unit>/mutants.tsv: N op killed|survived|invalid
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; U=$D/units; P=${1:-6}
one() {
  d=$1; . "$HERE/env.sh"
  [ "$(cut -f5 "$d/result.tsv" 2>/dev/null)" = PROVED ] || exit 0
  IFS=$'\t' read -r N name rest <"$d/result.tsv"
  first=$(head -1 "$d/fn.c"); : >"$d/mutants.tsv"
  for op in eq lt gt and add band shl ret; do
    W=$(mktemp -d); cp "$d/prelude.h" "$W/"
    awk -v first="$first" -v op="$op" '
      { L[NR] = $0 } END {
        s = 0; for (i = 1; i <= NR; i++) if (L[i] == first) s = i
        done = 0
        for (i = 1; i <= NR; i++) {
          t = L[i]
          if (i >= s && s > 0 && !done && t !~ /^[ \t]*(\/\*|\*|\/\/)/) {
            if (op == "eq" && sub(/==/, "!=", t)) done = 1
            else if (op == "lt" && match(t, /[^<]<[^<=]/)) { t = substr(t, 1, RSTART) "<=" substr(t, RSTART + 2); done = 1 }
            else if (op == "gt" && match(t, /[^>-]>[^>=]/)) { t = substr(t, 1, RSTART) ">=" substr(t, RSTART + 2); done = 1 }
            else if (op == "and" && sub(/&&/, "||", t)) done = 1
            else if (op == "add" && match(t, /[A-Za-z0-9_)\] ] ?\+ ?[A-Za-z0-9_(]/)) { p = index(substr(t, RSTART), "+") + RSTART - 1; if (substr(t, p - 1, 1) != "+" && substr(t, p + 1, 1) != "+") { t = substr(t, 1, p - 1) "-" substr(t, p + 1); done = 1 } }
            else if (op == "band" && match(t, /[^&]&[^&=]/) && t !~ /&[A-Za-z_]+[ ]*[,)]/) { t = substr(t, 1, RSTART) "|" substr(t, RSTART + 2); done = 1 }
            else if (op == "shl" && sub(/<</, ">>", t)) done = 1
            else if (op == "ret" && sub(/return \(?0L?\)?;/, "return 1;", t)) done = 1
            else if (op == "ret" && sub(/return \(?1L?\)?;/, "return 0;", t)) done = 1
          }
          print t
        }
        if (!done) print "NOMUTANT" > "/dev/stderr"
      }' "$d/unit.final.c" >"$W/unit.c" 2>"$W/err"
    if grep -q NOMUTANT "$W/err" || cmp -s "$d/unit.final.c" "$W/unit.c"; then rm -rf "$W"; continue; fi
    cd "$W"
    out=$(frama-c -kernel-warn-key annot-error=active -machdep gcc_x86_64 -wp -wp-no-rte -wp-cache none -wp-timeout 10 -wp-prover alt-ergo,z3,cvc5 -wp-fct "$name" unit.c 2>&1)
    pa=$(echo "$out" | sed -n 's/.*Proved goals: *\([0-9]*\) *\/ *\([0-9]*\).*/\1 \2/p' | tail -1)
    if [ -z "$pa" ]; then r=invalid; elif [ "${pa% *}" = "${pa#* }" ]; then r=survived; else r=killed; fi
    printf '%s\t%s\t%s\n' "$N" "$op" "$r" >>"$d/mutants.tsv"
    cd /; rm -rf "$W"
  done
}
export -f one; export HERE
ls -d "$U"/u* | xargs -P "$P" -I{} bash -c 'one {}'
cat "$U"/u*/mutants.tsv 2>/dev/null | cut -f3 | sort | uniq -c
