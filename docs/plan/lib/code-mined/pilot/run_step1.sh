#!/bin/bash
# run_step1.sh <A|C> [P]   Steps 1+2 on a sample: build units from real translation units, WP proof, bit-vector path for the unproved, replay of proved units, summary, export.
# Held by a window:  wsl -- bash run_step1.sh A 14   (see start_step1.cmd).  Resumable (units with results are skipped, replay.tsv too). STOP file: <rundir>/STOP.
# Work dir ~/kb/r<A|C> (WSL home, persistent); exported to docs/plan/lib/code-mined/pilot/data2/<A|C>/ at the end of every phase (small files only) and committed by export2.sh.
HERE=$(cd "$(dirname "$0")" && pwd); W=$1; P=${2:-14}
case "$W" in A) S=/mnt/c/ParanoidBSD/mined-data/pilot/sample.tsv ;; C) S=/mnt/c/ParanoidBSD/mined-data/pilot-C/sample.tsv ;; *) echo "A or C"; exit 2 ;; esac
R=$HOME/kb/r$W; mkdir -p "$R"; LOG=$R/run.log
say() { echo "$(date '+%F %T') $*" >>"$LOG"; }
export FC_TIMEOUT=${FC_TIMEOUT:-200}
bash "$HERE/build_tools.sh" 2>&1 | grep -E 'error' >>"$LOG"
say "start sample $W: $(wc -l <"$S") rows, P=$P"
bash "$HERE/run2.sh" "$S" "$R" "$P" >>"$LOG" 2>&1
say "phase 1+2 done: $(cat "$R"/u*/result.tsv | cut -f5 | sort | uniq -c | tr '\n' ' ')"
bash "$HERE/export2.sh" "$R" "$W" >>"$LOG" 2>&1
bash "$HERE/replay2.sh" "$R" 8 >>"$LOG" 2>&1
say "replay done"
bash "$HERE/export2.sh" "$R" "$W" >>"$LOG" 2>&1
touch "$R/DONE"; say "DONE"
