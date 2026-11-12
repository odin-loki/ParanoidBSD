#!/bin/bash
# dsv_watch.sh <budget-usd> [parallel]   Starts and supervises dsv_all.sh (the morph-dsv41flash-1017 manual-queue repair): checks every 60 s for the first hour, then every 10 minutes.
# Restarts it if it died or stalled (it resumes), at most 6 times an hour; stops on a credit/auth error (STOP file). Log: mined-data/dsv/watch.log, status.json beside it.
HERE=$(cd "$(dirname "$0")" && pwd); X=${DW:-/mnt/c/ParanoidBSD/mined-data/dsv}; export DW=$X; mkdir -p "$X"
BUDGET=${1:-40}; P=${2:-64}; LOG=$X/watch.log; LED=$X/ledger.txt
say() { echo "$(date '+%F %T') $*" | tee -a "$LOG"; }
alive() { pgrep -f "dsv_all.sh" > /dev/null; }
start() { rm -f "$X/DONE"; ( setsid nohup bash "$HERE/dsv_all.sh" "$BUDGET" "$P" >> "$X/repair.out" 2>&1 < /dev/null & ); say "dsv repair started (budget \$$BUDGET, parallel $P)"; }
spent() { awk '{i+=$1; o+=$2} END{printf "%.2f", (i*0.12+o*0.80)/1000000}' "$LED" 2>/dev/null; }
calls() { wc -l < "$LED" 2>/dev/null || echo 0; }
n=0; restarts=0; t0=$(date +%s); stall=0; last=$(calls); laste=$(wc -l < "$X/errors.log" 2>/dev/null || echo 0)
say "watcher started"; start
while true; do
  [ -f "$X/STOP" ] && { say "STOP file: watcher ends (spent \$$(spent))"; break; }
  if [ -f "$X/DONE" ] && ! alive; then say "finished: $(sort -u "$X/done.keys" | wc -l) records, spent \$$(spent)"; break; fi
  n=$((n+1)); interval=60; [ $n -gt 60 ] && interval=600
  [ $(( $(date +%s) - t0 )) -gt 3600 ] && { restarts=0; t0=$(date +%s); }
  cur=$(calls); e=$(wc -l < "$X/errors.log" 2>/dev/null || echo 0)
  if [ $((e - laste)) -gt 0 ] && tail -n $((e - laste)) "$X/errors.log" | grep -i 'attempt' | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key|forbidden'; then
    say "ALERT credit or auth error: $(tail -n $((e - laste)) "$X/errors.log" | grep -iE 'insufficient|credit|payment|billing|quota|40[123]|unauthorized|invalid|forbidden' | head -1 | cut -c1-200)"
    touch "$X/STOP"; say "ALERT stopped (xargs drains: the remaining records see STOP and exit). Top up, delete $X/STOP, run the launcher again (it resumes)."; break
  fi
  if alive; then
    if [ "$cur" -eq "$last" ]; then stall=$((stall+1)); else stall=0; fi
    [ $stall -ge 8 ] && { say "ALERT stall: no API call finished in $stall checks; restarting"; pkill -f dsv_all.sh; sleep 5; restarts=$((restarts+1)); start; stall=0; }
  elif [ ! -f "$X/DONE" ]; then
    say "ALERT dsv_all is not running: restarting ($((restarts+1)) this hour)"; restarts=$((restarts+1)); start
  fi
  [ $restarts -gt 6 ] && { say "ALERT too many restarts this hour: giving up"; break; }
  say "check $n: alive=$(alive && echo yes || echo no) calls=$cur (+$((cur-last))) records=$(sort -u "$X/done.keys" | wc -l) results=$(wc -l < "$X/dsv.jsonl" 2>/dev/null || echo 0) errors=$e (+$((e-laste))) failed=$(sort -u "$X/failed.txt" 2>/dev/null | wc -l) spent=\$$(spent)/\$$BUDGET restarts=$restarts"
  printf '{"time":"%s","records_done":%s,"spent_usd":%s,"budget_usd":%s,"restarts":%s}\n' "$(date -Is)" "$(sort -u "$X/done.keys" | wc -l)" "$(spent)" "$BUDGET" "$restarts" > "$X/status.json"
  last=$cur; laste=$e
  sleep $interval
done
