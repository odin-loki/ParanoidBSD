#!/bin/bash
# pilot_watch.sh [P_PROVE] [P_ANNOT]   Starts run_pilot.sh and supervises it: logs progress every 60 s to $D/watch.log, restarts the runner (it resumes) if it died without DONE/STOP, at most 5 times,
# and stops on STOP (credit error or USD cap, written by morph_call.sh). Holds the WSL session open for the whole run (LESSONS A.1). Does not kill anything it did not start.
HERE=$(cd "$(dirname "$0")" && pwd); D=${PILOT_DATA:-/mnt/c/ParanoidBSD/mined-data/pilot}; LOG=$D/watch.log
say() { echo "$(date '+%F %T') $*" | tee -a "$LOG"; }
rm -f "$D/DONE"  # (a runner that is already alive, per run.pid, is adopted, not restarted)
start() { ( setsid nohup bash "$HERE/run_pilot.sh" "$@" >>"$D/run.out" 2>&1 < /dev/null & echo $! >"$D/run.pid" ); say "runner started (pid $(cat "$D/run.pid"))"; }
alive() { [ -f "$D/run.pid" ] && kill -0 "$(cat "$D/run.pid")" 2>/dev/null; }
alive || start "$@"; restarts=0
while true; do
  p1=$(ls "$D"/units/u*/result.tsv 2>/dev/null | wc -l); p2=$(ls "$D"/units/u*/annot.tsv 2>/dev/null | wc -l)
  spent=$(awk -F'\t' '{s+=$8} END{printf "%.4f", s+0}' "$D/morph_ledger.tsv" 2>/dev/null)
  say "alive=$(alive && echo yes || echo no) proved-phase=$p1/200 annotated=$p2 spent_usd=$spent last: $(tail -n 1 "$D/run.log" 2>/dev/null | cut -c1-120)"
  [ -f "$D/DONE" ] && { say "DONE"; break; }
  [ -f "$D/STOP" ] && { say "STOP present: $(cat "$D/STOP")"; break; }
  if ! alive; then restarts=$((restarts+1)); [ $restarts -gt 5 ] && { say "too many restarts, giving up"; break; }; say "runner died; restarting ($restarts)"; start "$@"; fi
  sleep 60
done
