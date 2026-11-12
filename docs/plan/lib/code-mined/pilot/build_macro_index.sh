#!/bin/bash
# build_macro_index.sh  Index of single-line #defines in the headers of the mined trees (Linux, HBSD). Output: macros.uniq.tsv (name <TAB> full #define line) holding only the names that have exactly ONE
# distinct single-line definition across all headers. Ambiguous names (arch/config variants) are left unresolved on purpose: a proof must not depend on a guessed variant.
D=${D:-/mnt/c/ParanoidBSD/mined-data/pilot}
ROOTS="${ROOTS:-/home/odin/pbsd-ref/linux /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src}"
grep -rhIE --include='*.h' '^[ 	]*#[ 	]*define[ 	]+[A-Za-z_][A-Za-z0-9_]*' $ROOTS 2>/dev/null \
  | tr -d '\r' | awk '!/\\[ \t]*$/ { l=$0; sub(/^[ \t]*#[ \t]*define[ \t]+/,"",l); match(l,/^[A-Za-z_][A-Za-z0-9_]*/); n=substr(l,RSTART,RLENGTH); sub(/\/\*.*\*\//,"",l); sub(/\/\/.*$/,"",l); gsub(/[ \t]+$/,"",l); print n "\t#define " l }' \
  | sort -u -S 2G > $D/macros.all.tsv
awk -F'\t' '{ c[$1]++; d[$1]=$0 } END { for (n in c) if (c[n]==1) print d[n] }' $D/macros.all.tsv | sort > $D/macros.uniq.tsv
wc -l $D/macros.all.tsv $D/macros.uniq.tsv
