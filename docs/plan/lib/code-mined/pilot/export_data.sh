#!/bin/bash
# export_data.sh   Copy the small, committable evidence of the pilot from /mnt/c/ParanoidBSD/mined-data/pilot into pilot/data/ : sample, toolchain manifest, ledger (no key is ever in it), logs, the per-function result table,
# and for every unit the files needed to re-check or audit it (contract, extracted function, the exact proved unit, WP logs, model replies, Lean files). The macro index (hundreds of MB) is not copied; its sha256 is in toolchain.txt.
D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; HERE=$(cd "$(dirname "$0")" && pwd); O=$HERE/data; mkdir -p "$O/units"
cp "$D/sample.tsv" "$D/toolchain.txt" "$D/morph_ledger.tsv" "$O/" 2>/dev/null
for f in run.log watch.log run1.log lean.log; do [ -f "$D/$f" ] && grep -v '^test$' "$D/$f" >"$O/$f"; done
bash "$HERE/summarize.sh" >/dev/null
for d in "$D"/units/u*; do
  n=$(basename "$d"); mkdir -p "$O/units/$n"
  for f in meta.tsv result.tsv annot.tsv prism.tsv replay.tsv lean.tsv contract.acsl fn.c resolved.h resolved.names unit.final.c prelude.h wp.A.log wp.B.log; do [ -f "$d/$f" ] && cp "$d/$f" "$O/units/$n/"; done
  # model traffic: replies and extracted Lean/annotated code, not the prompts' duplicate source
  for f in "$d"/att*.fn.c "$d"/att*.result.tsv "$d"/att*.guard.txt "$d"/lean*.lean "$d"/lean*.out.txt; do [ -f "$f" ] && cp "$f" "$O/units/$n/"; done
  [ -f "$d/fn.ann.c" ] && cp "$d/fn.ann.c" "$O/units/$n/"
done
du -sh "$O" | cut -f1
