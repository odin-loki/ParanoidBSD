#!/bin/bash
# repair_watch.sh <budget-usd> [parallel]   Starts and supervises repair_all.sh: checks every 60 s for the first hour, then every 10 minutes.
# Restarts it if it died or stalled (it resumes), at most 6 times an hour; stops everything on a credit/auth error (writes STOP).
# Log: /mnt/c/ParanoidBSD/mined-data/verify/repair/watch.log ; status.json beside it.
HERE=$(cd "$(dirname "$0")" && pwd); X=${RP:-/mnt/c/ParanoidBSD/mined-data/verify/repair}; mkdir -p "$X"
BUDGET=${1:-60}; P=${2:-48}; LOG=$X/watch.log; LED=$X/ledger.txt
say() { echo "$(date '+%F %T') $*" | tee -a "$LOG"; }
alive() { pgrep -f "repair_all.sh" > /dev/null; }
start() { rm -f "$X/DONE"; ( setsid nohup bash "$HERE/repair_all.sh" 999999 "$BUDGET" "$P" >> "$X/repair.out" 2>&1 < /dev/null & ); say "repair started (budget \$$BUDGET, parallel $P)"; }
spent() { awk '{i+=$1; o+=$2} END{printf "%.2f", (i*0.047+o*0.268)/1000000}' "$LED" 2>/dev/null; }
n=0; restarts=0; t0=$(date +%s); stall=0
last=$(wc -l < "$X/repair.jsonl" 2>/dev/null || echo 0); laste=$(wc -l < "$X/errors.log" 2>/dev/null || echo 0)
say "watcher started"; start
while true; do
  [ -f "$X/STOP" ] && { say "STOP file: watcher ends"; break; }
  if [ -f "$X/DONE" ] && ! alive; then say "finished: $(wc -l < "$X/repair.jsonl") records, spent \$$(spent)"; break; fi
  n=$((n+1)); interval=60; [ $n -gt 60 ] && interval=600
  [ $(( $(date +%s) - t0 )) -gt 3600 ] && { restarts=0; t0=$(date +%s); }
  cur=$(wc -l < "$X/repair.jsonl" 2>/dev/null || echo 0); e=$(wc -l < "$X/errors.log" 2>/dev/null || echo 0)
  if [ $((e - laste)) -gt 0 ] && tail -n $((e - laste)) "$X/errors.log" | grep -i '"error"' | grep -i "\"error\"" | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key|forbidden'; then
    say "ALERT credit or auth error: $(tail -n $((e - laste)) "$X/errors.log" | grep -iE 'insufficient|credit|payment|billing|quota|40[123]|unauthorized|invalid|forbidden' | head -1 | cut -c1-200)"
    touch "$X/STOP"; pkill -f repair_all.sh; pkill -f repair_chunk.sh; say "ALERT stopped. Top up, delete $X/STOP, run the launcher again (it resumes)."; break
  fi
  if alive; then
    if [ -f "$X/BUILDING" ]; then stall=0                       # building the todo list: nothing is expected to finish
    elif [ "$cur" -eq "$last" ]; then stall=$((stall+1)); else stall=0; fi
    [ $stall -ge 6 ] && { say "ALERT stall: nothing finished in $stall checks; restarting"; pkill -f repair_all.sh; pkill -f repair_chunk.sh; sleep 5; restarts=$((restarts+1)); start; stall=0; }
  elif [ ! -f "$X/DONE" ]; then
    say "ALERT repair is not running: restarting ($((restarts+1)) this hour)"; restarts=$((restarts+1)); start
  fi
  [ $restarts -gt 6 ] && { say "ALERT too many restarts this hour: giving up"; break; }
  if ! tail -n 20 "$X/repair.jsonl" 2>/dev/null | jq -e . > /dev/null 2>&1; then say "ALERT corrupt lines at the end of repair.jsonl"; fi
  say "check $n: alive=$(alive && echo yes || echo no) records=$cur (+$((cur-last)), $(( (cur-last)*60/interval ))/min) errors=$e (+$((e-laste))) failed=$(sort -u "$X/failed.txt" 2>/dev/null | wc -l) spent=\$$(spent)/\$$BUDGET restarts=$restarts"
  printf '{"time":"%s","records":%s,"spent_usd":%s,"budget_usd":%s,"restarts":%s}\n' "$(date -Is)" "$cur" "$(spent)" "$BUDGET" "$restarts" > "$X/status.json"
  last=$cur; laste=$e
  sleep $interval
done
