#!/bin/bash
# post_core.sh [P]   Follow-up phases of the core run, after prove_core.sh is DONE: pointer/loop bit-vector pass for every unproved unit (rebv.sh), summary, replay of the new proofs,
# mutation test and contract-strength of the whole proved set, S5 audit sample (400), snapshot commit.  Resumable (each tool skips finished units).
HERE=$(cd "$(dirname "$0")" && pwd); P=${1:-14}; R=$HOME/kb/rCore; OUT=/mnt/c/ParanoidBSD/mined-data/prove-core
say() { echo "$(date '+%F %T') post: $*" | tee -a "$R/run.log" >>"$OUT/run.log"; }
say "rebv"; bash "$HERE/rebv.sh" "$R" "$P"
bash "$HERE/summarize2.sh" "$R" "core run after bit-vector pass" >"$OUT/summary.md" 2>&1
say "replay"; rm -f "$R"/u*/replay.tsv; bash "$HERE/replay2.sh" "$R" 8 >>"$R/run.log" 2>&1
say "mutation"; bash "$HERE/mutate2.sh" "$R" "$P" >>"$R/run.log" 2>&1
say "strength"; bash "$HERE/strength2.sh" "$R" >>"$R/run.log" 2>&1
bash "$HERE/sync_core.sh" "$R" "$OUT" >>"$R/run.log" 2>&1
cp "$R/strength.tsv" "$R/mutation_summary.tsv" "$OUT/" 2>/dev/null
say "audit sample"; SAMPLE="$R/sample.tsv" bash "$HERE/audit_sample.sh" "$R" "$OUT/audit-sample" 400 >>"$R/run.log" 2>&1
mkdir -p "$HERE/data2/core"; cp "$OUT/strength.tsv" "$OUT/mutation_summary.tsv" "$HERE/data2/core/" 2>/dev/null; gzip -f "$HERE/data2/core/strength.tsv" "$HERE/data2/core/mutation_summary.tsv"
bash "$HERE/commit_core.sh" >>"$R/run.log" 2>&1
touch "$R/POST-DONE"; say "POST-DONE"
