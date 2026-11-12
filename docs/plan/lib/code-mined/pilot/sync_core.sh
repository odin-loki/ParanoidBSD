#!/bin/bash
# sync_core.sh <rundir> <outdir>   Copy the small result files of every unit to the persistent output directory (Windows side) and write labels.tsv, summary.md, repair_list.tsv there.
# Copied per unit: meta/contract/result/replay/bv/mutation files, tu.how, signature; the proved unit file gzipped. Not copied: logs, preprocessed units, macro tables.
# 13 Nov 2026: per-unit work runs 32 at a time (the copy goes over the slow Windows file share and ran one unit at a time: ~35 minutes per 1000 units, the prover idle meanwhile).
R=$1; O=$2; HERE=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$O/units"
export R O
sync_one() {
  u=$1; cd "$R" || exit 0
  [ -f "$u/result.tsv" ] || exit 0
  d=$O/units/$u
  # skip units already synced and unchanged (result.tsv newer than the marker)
  if [ -f "$d/.synced" ] && [ ! "$u/result.tsv" -nt "$d/.synced" ] && [ ! "$u/result.bv.tsv" -nt "$d/.synced" ] && [ ! "$u/replay.tsv" -nt "$d/.synced" ] && [ ! "$u/mutation.tsv" -nt "$d/.synced" ] && [ ! "$u/result.bv2.tsv" -nt "$d/.synced" ]; then exit 0; fi
  mkdir -p "$d"
  for f in meta.tsv contract.acsl contract.exp result.tsv result.bv.tsv result.bv2.tsv replay.tsv mutation.tsv tu.how machdep unit.c.sig unit.c.callees bv.cex; do [ -f "$u/$f" ] && cp "$u/$f" "$d/"; done
  [ -f "$u/unit.final.c" ] && gzip -c "$u/unit.final.c" >"$d/unit.final.c.gz"
  touch "$d/.synced"
}
export -f sync_one
( cd "$R" && ls -d u* | xargs -P 32 -I{} bash -c 'sync_one {}' )
bash "$HERE/summarize2.sh" "$R" "core run" >"$O/summary.md" 2>&1
cp "$R/labels.tsv" "$R/repair_list.tsv" "$O/" 2>/dev/null
echo "synced $(ls "$O/units" | wc -l) units"
