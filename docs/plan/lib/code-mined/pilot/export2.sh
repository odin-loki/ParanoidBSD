#!/bin/bash
# export2.sh <rundir> <A|C|name>   Copy the small, reviewable files of a run into docs/plan/lib/code-mined/pilot/data2/<name>/ (committed): per unit meta, contracts, results,
# replay result, assumptions, signature, WP/CBMC logs (gzip), the proved unit file gzipped. Not copied: preprocessed translation units, defs.txt.gz (macro tables), raw logs > 200 kB.
# Also writes summary.md, labels.tsv and repair_list.tsv (summarize2.sh).
HERE=$(cd "$(dirname "$0")" && pwd); R=$1; N=$2; D=$HERE/data2/$N
mkdir -p "$D/units"
for u in "$R"/u*; do
  b=$(basename "$u"); o=$D/units/$b; mkdir -p "$o"
  for f in meta.tsv contract.acsl contract.exp result.tsv result.bv.tsv replay.tsv tu.how kconfig.txt machdep bsd.stubs slice.err unit.c.callees unit.c.sig bv.cex; do [ -f "$u/$f" ] && cp "$u/$f" "$o/"; done
  for f in unit.final.c unit.c wp.A.log wp.B.log wp.T.log bv.A.log bv.B.log bv.h.c parse.log; do [ -f "$u/$f" ] && [ "$(stat -c %s "$u/$f")" -lt 3000000 ] && gzip -c "$u/$f" >"$o/$f.gz"; done
  rm -f "$o/unit.c.gz"; [ -f "$o/unit.final.c.gz" ] || { [ -f "$u/unit.c" ] && gzip -c "$u/unit.c" >"$o/unit.c.gz"; }
done
bash "$HERE/summarize2.sh" "$R" "sample $N" >"$D/summary.md" 2>&1
cp "$R/labels.tsv" "$R/repair_list.tsv" "$D/" 2>/dev/null
echo "exported $(ls "$D/units" | wc -l) units to $D"
