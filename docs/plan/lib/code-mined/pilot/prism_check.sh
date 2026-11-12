#!/bin/bash
# prism_check.sh <unitdir>   Second, independent engine: run PRISM (contracts + wp + pir stages, its own Z3-based WP and bit-precise BMC) on the same unit that Frama-C WP proved.
# PRISM spells the result `result` (not `\result`) and never reports an unconditional PROVED when the contract has a requires (PROVED-ASSUMING), so this is a cross-check, not the label.
# Writes <unit>/prism.tsv: N name wp_status contracts_status pir_status seconds   ("agree" = wp and contracts both PROVED*)
U=$1; cd "$U" || exit 2
IFS=$'\t' read -r N name kind cls file lines hc hf hp mstat <meta.tsv
[ -f unit.final.c ] || exit 0
W=$(mktemp -d "$U/.prism.XXXXXX"); mkdir -p "$W/scope"
cp prelude.h "$W/scope/"
sed -E 's/\\result/result/g' unit.final.c >"$W/scope/unit.c"
t0=$(date +%s.%N)
( cd "$W/scope" && timeout 300 ${PRISM_BIN:-$HOME/PRISM/build/prism} . --no-llm --jobs 2 --stage inventory,classify,contracts,wp,pir --out "$W/out" --unwind 32 >"$W/prism.log" 2>&1 )
secs=$(awk -v a="$t0" -v b="$(date +%s.%N)" 'BEGIN{printf "%.1f", b-a}')
g() { jq -r --arg s "$1" --arg f "$name" '[.stages[].findings[] | select(.stage==$s and .function==$f) | .status] | if length==0 then "none" else join("+") end' "$W/out/report.json" 2>/dev/null || echo error; }
printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$N" "$name" "$(g wp)" "$(g contracts)" "$(g pir)" "$secs" >prism.tsv
cp "$W/out/report.json" prism.report.json 2>/dev/null; rm -rf "$W"
