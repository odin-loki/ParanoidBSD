#!/bin/bash
# cl_watch.sh <job-name>...   Starts and supervises cl_all.sh for each job (mined-data/claude/jobs/<name>/): checks every 60 s (every 10 min after the first hour),
# restarts a dead job (it resumes), at most 6 times an hour per job; stops everything (STOP file) on a credit/auth ALERT. Log: mined-data/claude/watch.log, status.json beside it.
HERE=$(cd "$(dirname "$0")" && pwd); . "$HERE/cl_lib.sh"; LOG=$CLD/watch.log; JOBS=("$@")
say() { echo "$(date '+%F %T') $*" | tee -a "$LOG"; }
alive() { pgrep -f "cl_all.sh $CLD/jobs/$1\$" > /dev/null; }
start() { rm -f "$CLD/jobs/$1/STOP"; ( setsid nohup bash "$HERE/cl_all.sh" "$CLD/jobs/$1" >> "$CLD/jobs/$1/out.log" 2>&1 < /dev/null & ); say "job $1 started"; }
spent() { ledger_cost "$CLD"/test/*/ledger.txt "$CLD"/jobs/*/ledger.txt; }
declare -A RS; for j in "${JOBS[@]}"; do RS[$j]=0; done
t0=$(date +%s); n=0
say "watcher started for: ${JOBS[*]} (spent \$$(spent))"
rm -f "$CLD/STOP"; for j in "${JOBS[@]}"; do start "$j"; done
while true; do
  n=$((n+1)); interval=60; [ $n -gt 60 ] && interval=600
  [ $(( $(date +%s) - t0 )) -gt 3600 ] && { for j in "${JOBS[@]}"; do RS[$j]=0; done; t0=$(date +%s); }
  [ -f "$CLD/STOP" ] && { say "global STOP file: watcher ends (spent \$$(spent))"; break; }
  active=0
  for j in "${JOBS[@]}"; do
    J=$CLD/jobs/$j
    if grep -q 'ALERT credit or auth' "$J/job.log" 2>/dev/null && [ -f "$J/STOP" ]; then say "ALERT in job $j: stopping all jobs (top up, delete STOP files, run start_claude.cmd again: it resumes)"; touch "$CLD/STOP"; continue 2; fi
    [ -f "$J/DONE" ] && continue
    if [ -f "$J/STOP" ]; then alive "$j" && active=1; continue; fi
    active=1
    if ! alive "$j"; then
      if [ "${RS[$j]}" -ge 6 ]; then say "ALERT job $j: too many restarts this hour, not restarting"; continue; fi
      RS[$j]=$((RS[$j]+1)); say "ALERT job $j is not running: restarting (${RS[$j]} this hour)"; start "$j"
    fi
  done
  say "check $n: spent \$$(spent) | $(for j in "${JOBS[@]}"; do J=$CLD/jobs/$j; printf '%s done=%s inflight=%s gaveup=%s; ' "$j" "$(sort -u "$J/done.keys" 2>/dev/null | wc -l)" "$(ls -d "$J"/b*/ 2>/dev/null | while read -r b; do [ -f "$b/id" ] && [ ! -f "$b/collected" ] && echo x; done | wc -l)" "$(sort -u "$J/gaveup.keys" 2>/dev/null | wc -l)"; done)"
  printf '{"time":"%s","spent_usd":%s}\n' "$(date -Is)" "$(spent)" > "$CLD/status.json"
  [ $active -eq 0 ] && { say "all jobs finished or stopped: watcher ends"; break; }
  sleep $interval
done
