#!/bin/bash
# prove_core.sh [P] [CHUNK]   FULL core-tier proving run: every function of cands.tsv (24,825 verified, core-kind, with a formal contract) through the step 1+2 pipeline
# (real translation unit -> WP -> CBMC bit-vector path for the unproved), in chunks of CHUNK rows (default 1000) so that milestones and snapshots are regular.
#   * order: seeded shuffle of cands.cls.tsv (a partial run is a representative sample, not "all arith-helpers first")
#   * work dir ~/kb/rCore (WSL home, fast); after every chunk: sync_core.sh copies the small result files to /mnt/c/ParanoidBSD/mined-data/prove-core (persistent) and
#     writes labels.tsv / summary.md there; every 4 chunks the snapshot (labels, summary, repair list) is committed and pushed (docs/plan/lib/code-mined/pilot/data2/core/)
#   * resumable: a unit with result.tsv (and result.bv.tsv where due) is skipped; STOP file in ~/kb/rCore stops it after the current unit
#   * 25% milestones are written to <out>/MILESTONES.md
#   * at the end: replay2.sh on every proved unit (clean directory, cache off)
# Run held:  wsl -- bash prove_core.sh 14        (see watch_core.sh, start_core.cmd)
HERE=$(cd "$(dirname "$0")" && pwd); P=${1:-14}; CH=${2:-1000}
R=$HOME/kb/rCore; OUT=/mnt/c/ParanoidBSD/mined-data/prove-core; mkdir -p "$R" "$OUT"
LOG=$R/run.log; say() { echo "$(date '+%F %T') $*" | tee -a "$LOG" >>"$OUT/run.log"; }
export FC_TIMEOUT=${FC_TIMEOUT:-200}
echo $$ >"$R/run.pid"
bash "$HERE/build_tools.sh" 2>&1 | grep -E 'error' >>"$LOG"
S=$R/sample.tsv
if [ ! -s "$S" ]; then
  awk -f "$HERE/cls.awk" /mnt/c/ParanoidBSD/mined-data/pilot/cands.tsv >"$R/cands.cls.tsv"
  shuf --random-source=<(yes pbsd-core-run-1) "$R/cands.cls.tsv" >"$S.part" && mv "$S.part" "$S"
fi
cp "$S" "$OUT/sample.tsv"
n=$(wc -l <"$S"); say "core run: $n functions, P=$P, chunk=$CH"
last=0; chunkno=0
while [ "$last" -lt "$n" ]; do
  [ -f "$R/STOP" ] && { say "STOP file present"; break; }
  a=$((last+1)); z=$((last+CH)); [ "$z" -gt "$n" ] && z=$n
  say "chunk $a-$z start"
  bash "$HERE/run2.sh" "$S" "$R" "$P" "$a" "$z" >>"$LOG" 2>&1
  last=$z; chunkno=$((chunkno+1))
  # the preprocessed-unit cache is only a cache: keep it under 120 GB (oldest first)
  tuk=$(du -sk "$HOME/kb/tu" 2>/dev/null | cut -f1); if [ "${tuk:-0}" -gt 125000000 ]; then ls -tr "$HOME/kb/tu"/*.i 2>/dev/null | head -n 3000 | xargs -r rm -f; say "tu cache trimmed"; fi
  bash "$HERE/sync_core.sh" "$R" "$OUT" >>"$LOG" 2>&1
  say "chunk $a-$z done: $(cat "$R"/u*/result.tsv 2>/dev/null | cut -f5 | sort | uniq -c | tr '\n' ' ')"
  # milestone every 25%
  pct=$((100*last/n)); prev=$((100*(last-CH)/n)); [ "$((pct/25))" -gt "$((prev/25))" ] && { { echo "## ${pct}% ($last of $n) at $(date '+%F %T')"; echo; cat "$OUT/summary.md"; echo; } >>"$OUT/MILESTONES.md"; say "milestone ${pct}%"; }
  if [ $((chunkno % 4)) -eq 0 ] || [ "$last" -ge "$n" ]; then bash "$HERE/commit_core.sh" >>"$LOG" 2>&1; fi
done
if [ "$last" -ge "$n" ] && [ ! -f "$R/STOP" ]; then
  say "all chunks done; replay"
  bash "$HERE/replay2.sh" "$R" 8 >>"$LOG" 2>&1
  bash "$HERE/sync_core.sh" "$R" "$OUT" >>"$LOG" 2>&1
  bash "$HERE/commit_core.sh" >>"$LOG" 2>&1
  touch "$R/DONE"; say "DONE"
fi
