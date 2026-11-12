#!/bin/bash
# strength2.sh <rundir>   Classify the contract of every proved unit (PROVED / PROVED-ASSUMING by WP or CBMC, and BOUNDED) as
#   trivial    the ensures says nothing about \result
#   range      \result only compared with constants / enumerators (no parameter of the function in a clause that mentions \result)
#   functional some ensures clause equates (== or <==>) or orders \result against an expression that contains a parameter of the function
# Stricter than the pilot heuristic (which counted any identifier, so a macro name or a cast made a range contract "functional"). Writes <rundir>/strength.tsv: N name kind label strength
R=$1; cd "$R" || exit 2; : >strength.tsv
awk -F'\t' '$6 ~ /^PROVED|^BOUNDED/ {print $1}' labels.tsv | while read -r n; do
  d=u$(printf %03d "$n"); [ -f "$d/contract.exp" ] || d=u$n
  [ -f "$d/contract.exp" ] || continue
  params=$(tail -n +2 "$d/unit.c.sig" 2>/dev/null | cut -f2 | tr '\n' ' ')
  s=$(sed -E 's#^[[:space:]]*/\*@##; s#\*/[[:space:]]*$##' "$d/contract.exp" | tr ';' '\n' | awk -v params="$params" '
    BEGIN { np = split(params, P, " ") ; res = 0; fn = 0 }
    /ensures/ { sub(/^[ \t]*ensures/, ""); if ($0 ~ /\\result/ || $0 ~ /(^|[^A-Za-z0-9_])result([^A-Za-z0-9_]|$)/) { res = 1
        t = $0; gsub(/\\result/, " ", t); for (i = 1; i <= np; i++) { if (P[i] != "" && t ~ ("(^|[^A-Za-z0-9_])" P[i] "([^A-Za-z0-9_]|$)")) fn = 1 } } }
    END { print (res == 0 ? "trivial" : (fn ? "functional" : "range")) }')
  printf '%s\t%s\t%s\t%s\t%s\n' "$n" "$(awk -F'\t' -v n=$n '$1==n{print $2}' labels.tsv)" "$(awk -F'\t' -v n=$n '$1==n{print $3}' labels.tsv)" "$(awk -F'\t' -v n=$n '$1==n{print $6}' labels.tsv)" "$s" >>strength.tsv
done
echo "strength of the proved contracts: $(cut -f5 strength.tsv | sort | uniq -c | tr '\n' ' ')"
