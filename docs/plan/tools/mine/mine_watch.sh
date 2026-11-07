#!/bin/bash
# mine_watch.sh   Watches mine_all.sh: every 60 s for the first hour, then every 10 minutes, until DONE-all or a STOP file exists.
# Writes /mnt/c/ParanoidBSD/mined-data/watch.log (one status line per check, "ALERT ..." lines for anything wrong) and status.json.
# It restarts the miner if it died or stalled (resumable, at most 6 restarts per hour) and STOPS it on a credit/auth error so it never hammers the API.
D=/mnt/c/ParanoidBSD/mined-data
HERE=$(cd "$(dirname "$0")" && pwd)
LOG=$D/watch.log; LED=$D/ledger.txt; mkdir -p "$D"
TIERS=${MINE_TIERS:-"B C"}; BUDGET=${MINE_BUDGET:-1000000}; P=${MINE_P:-12}
say()   { echo "$(date '+%F %T') $*" | tee -a "$LOG"; }
alert() { echo "$(date '+%F %T') ALERT $*" | tee -a "$LOG"; }
alive() { pgrep -f "mine_all.sh" > /dev/null; }
start_miner() { ( cd "$D" && setsid nohup bash "$HERE/mine_all.sh" "$TIERS" "$BUDGET" >> "$D/miner.out" 2>&1 < /dev/null & ); say "miner started (tiers $TIERS, parallel $P)"; }
spent() { awk '{i+=$1; o+=$2} END{printf "%.2f", (i*0.047+o*0.268)/1000000}' "$LED" 2>/dev/null; }

last_lines=$(wc -l < "$LED" 2>/dev/null || echo 0); last_fail=$(wc -l < "$D/failed.txt" 2>/dev/null || echo 0); last_err=$(wc -l < "$D/errors.log" 2>/dev/null || echo 0)
stall=0; restarts=0; hour_start=$(date +%s); n=0
say "watcher started"
while true; do
  [ -f "$D/DONE-all" ] && { say "DONE-all found: finished. spent \$$(spent), chunks in ledger $(wc -l < "$LED")"; break; }
  [ -f "$D/STOP" ] && { say "STOP file found: watcher ending"; break; }
  n=$((n+1)); now=$(date +%s)
  [ $((now - hour_start)) -gt 3600 ] && { restarts=0; hour_start=$now; }
  lines=$(wc -l < "$LED" 2>/dev/null || echo 0); fails=$(wc -l < "$D/failed.txt" 2>/dev/null || echo 0); errs=$(wc -l < "$D/errors.log" 2>/dev/null || echo 0)
  delta=$((lines - last_lines)); fdelta=$((fails - last_fail)); edelta=$((errs - last_err))
  interval=60; [ $n -gt 60 ] && interval=600
  rate=$((delta * 60 / interval))
  # 1. credit / auth errors in the NEW part of errors.log: stop the miner, do not retry
  if [ "$edelta" -gt 0 ]; then
    new=$(tail -n "$edelta" "$D/errors.log")
    if echo "$new" | grep -qiE 'insufficient|credit|payment|billing|quota exceeded|"code":"?40[123]|unauthorized|invalid api key|forbidden'; then
      alert "CREDIT or AUTH error from the API: $(echo "$new" | grep -iE 'insufficient|credit|payment|billing|quota|40[123]|unauthorized|invalid|forbidden' | head -1 | cut -c1-200)"
      touch "$D/STOP"; pkill -f mine_all.sh; pkill -f mine_chunk.sh; alert "miner stopped (STOP file written). Top up and delete $D/STOP, then start again: it resumes."
      break
    fi
    n429=$(echo "$new" | grep -ciE '429|rate.?limit|too many'); [ "$n429" -gt 0 ] && say "note: $n429 rate-limit responses this minute (the miner backs off by itself)"
  fi
  # 2. liveness and stall
  if alive; then
    # while the chunk list is being built (a few minutes, reads every file once) nothing is expected to finish
    if ls "$D"/chunks-tier*.txt.part > /dev/null 2>&1; then stall=0
    elif [ "$delta" -eq 0 ]; then stall=$((stall+1)); else stall=0; fi
    [ "$stall" -ge 5 ] && { alert "STALL: no chunk finished for $stall checks while the miner is alive"; if [ "$restarts" -lt 6 ]; then pkill -f mine_all.sh; pkill -f mine_chunk.sh; sleep 3; restarts=$((restarts+1)); start_miner; stall=0; else alert "too many restarts this hour: leaving it stopped"; break; fi; }
  else
    alert "the miner is not running (no DONE-all): restarting ($((restarts+1)) this hour)"
    if [ "$restarts" -lt 6 ]; then restarts=$((restarts+1)); start_miner; else alert "too many restarts this hour: giving up"; break; fi
  fi
  # 3. failure rate
  if [ "$delta" -gt 0 ] && [ "$fdelta" -gt $((delta / 4 + 3)) ]; then alert "HIGH FAILURE RATE: $fdelta chunks failed against $delta finished in the last interval"; fi
  # 4. data validity: the newest lines of the live results file must parse
  f=$(ls -t "$D"/tier*.jsonl 2>/dev/null | head -1)
  if [ -n "$f" ] && ! tail -n 30 "$f" | jq -e . > /dev/null 2>&1; then alert "CORRUPT lines at the end of $f"; fi
  # 5. disk (MB free on C:)
  free=$(df -m /mnt/c | awk 'NR==2{print $4}'); [ "$free" -lt 2000 ] && alert "DISK LOW: only ${free} MB free on C:"
  # 6. status
  prog=""; for t in B C; do [ -f "$D/progress-tier$t.txt" ] && prog="$prog tier$t $(cat "$D/progress-tier$t.txt")/$(wc -l < "$D/chunks-tier$t.txt" 2>/dev/null)"; done
  say "check $n: alive=$(alive && echo yes || echo no) chunks_done=$lines (+$delta, ${rate}/min) fails=$fails (+$fdelta) spent=\$$(spent) disk_free=${free}MB restarts=$restarts |$prog"
  printf '{"time":"%s","chunks_done":%s,"rate_per_min":%s,"failed":%s,"spent_usd":%s,"disk_free_mb":%s,"restarts":%s}\n' "$(date -Is)" "$lines" "$rate" "$fails" "$(spent)" "$free" "$restarts" > "$D/status.json"
  last_lines=$lines; last_fail=$fails; last_err=$errs
  sleep "$interval"
done
