#!/bin/bash
# watch_core.sh [P]   Held window for the core run:  wsl -- bash watch_core.sh 14   (started with run_in_background / a held console, NOT cmd /k)
# Restarts prove_core.sh (resumable) if it exits before DONE/STOP, then runs the follow-up phases (bit-vector for pointers, mutation test, audit sample) when present.
HERE=$(cd "$(dirname "$0")" && pwd); P=${1:-14}; R=$HOME/kb/rCore; mkdir -p "$R"
tries=0
while [ ! -f "$R/DONE" ] && [ ! -f "$R/STOP" ] && [ $tries -lt 20 ]; do
  bash "$HERE/prove_core.sh" "$P"; tries=$((tries+1)); sleep 30
done
echo "$(date '+%F %T') watcher exit tries=$tries done=$([ -f $R/DONE ] && echo yes || echo no)" >>"$R/run.log"
[ -f "$R/DONE" ] && [ -x "$HERE/post_core.sh" -o -f "$HERE/post_core.sh" ] && bash "$HERE/post_core.sh" "$P" >>"$R/run.log" 2>&1
