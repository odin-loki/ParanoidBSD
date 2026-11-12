#!/bin/bash
# kill_stuck_units.sh   While the core run is draining (STOP set), end unit workers that are stuck in the old tu_bsd.sh `find` (older than 30 minutes in mkunit2.sh).
# Only the worker's own process tree is ended (never xargs); the unit has no result.tsv yet, so the resumed run rebuilds it with the fast lookup.
for p in $(ps -eo pid,etimes,args | awk '$2 > 1800 && /mkunit2\.sh/ && !/awk/ {print $1}'); do
  w=$(ps -o ppid= -p "$p" | tr -d ' ')           # the "bash -c" worker that runs mkunit2
  u=$(ps -o args= -p "$p" | sed -E 's/.*mkunit2.sh ([0-9]+).*/\1/')
  d="$HOME/kb/rCore/u$(printf %03d "$u")"
  if [ -f "$d/result.tsv" ]; then echo "unit $u has a result, skipped"; continue; fi
  echo "ending stuck worker for unit $u (pid $w)"
  pkill -TERM -P "$p" 2>/dev/null; pkill -TERM -P "$w" 2>/dev/null; kill -TERM "$p" 2>/dev/null
  for c in $(pgrep -P "$p" 2>/dev/null); do kill -TERM "$c" 2>/dev/null; done
  kill -TERM "$w" 2>/dev/null
done
sleep 3
ps -eo pid,etimes,args | grep -E 'mkunit2|tu_bsd' | grep -v grep | cut -c1-100
