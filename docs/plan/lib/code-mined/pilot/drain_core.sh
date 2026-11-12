#!/bin/bash
# drain_core.sh   Stop the core run cleanly (STOP file: the unit workers finish the unit they are on and exit), wait until the run is fully down, remove STOP.
# Used to restart the run with more workers (the run is resumable: units with result.tsv are skipped). Never kills xargs.
R=$HOME/kb/rCore
touch "$R/STOP"
echo "$(date '+%F %T') drain: STOP set"
for i in $(seq 1 240); do
  n=$(ps -eo args | grep -E 'watch_core.sh|prove_core.sh|run2.sh|mkunit2.sh|prove_unit2.sh|bv_unit.sh|sync_core.sh|summarize2.sh' | grep -v grep | wc -l)
  [ "$n" -eq 0 ] && break
  sleep 15
done
echo "$(date '+%F %T') drain: remaining core processes=$n"
if [ "$n" -eq 0 ]; then rm -f "$R/STOP"; echo "$(date '+%F %T') drain: STOP removed, ready to restart"; else echo "drain: not fully down, STOP left in place"; fi
