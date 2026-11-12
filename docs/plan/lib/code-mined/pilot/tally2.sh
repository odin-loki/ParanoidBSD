#!/bin/bash
# tally2.sh <rundir> [tree]   quick tally of a run2.sh output: status counts and the most frequent causes
D=$1; T=${2:-.}
cd "$D" || exit 2
echo "== status"; cat u*/result.tsv | awk -F'\t' -v t="$T" 't=="." || $6==t {print $5}' | sort | uniq -c
echo "== unbuilt causes"
cat u*/result.tsv | awk -F'\t' -v t="$T" '($6==t||t==".") && $5=="UNBUILT"{c=$13; sub(/Location: "[^"]*", /,"",c); gsub(/line[s]? [0-9-]+,? ?/,"",c); gsub(/unit.c:[0-9]+/,"unit.c",c); print substr(c,1,100)}' | sort | uniq -c | sort -rn | head -${TOP:-25}
echo "== meta not ok"
awk -F'\t' -v t="$T" '($6==t||t==".") && $7!="ok"{print $7, substr($8,1,90)}' u*/meta.tsv | sort | uniq -c | sort -rn | head -12
