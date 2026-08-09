#!/bin/bash
# Outer supervisor: keeps pbsd_driver alive, refreshes auth, pushes periodically.
export PATH="$HOME/.local/bin:$PATH"
unset CURSOR_API_KEY
LOG="$HOME/pbsd_watchdog.log"
PUSH_INTERVAL="${PUSH_INTERVAL:-1800}"
LOCK="$HOME/pbsd_watchdog.lock"
last_push=0

log() { echo "$(date -u +%FT%TZ) $*" | tee -a "$LOG"; }

push_if_needed() {
  now=$(date +%s)
  if [ $((now - last_push)) -lt "$PUSH_INTERVAL" ]; then return 0; fi
  log "running github push"
  bash "$HOME/push_github.sh" >>"$LOG" 2>&1 || log "github push failed"
  last_push=$now
}

start_driver() {
  bash "$HOME/sync_cursor_auth.sh" 2>/dev/null || true
  src="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd.py"
  tmp="$(mktemp)"
  if [ -f "$src" ]; then
    cp -f "$src" "$tmp"
    sed -i 's/\r$//' "$tmp"
    if python3 -m py_compile "$tmp" 2>/dev/null; then
      mv "$tmp" ~/pbsd/pbsd.py
    else
      log "pbsd.py from Windows failed py_compile — keeping existing ~/pbsd/pbsd.py"
      rm -f "$tmp"
    fi
  fi
  if [ -n "${RESTART_FORCE:-}" ]; then
    pkill -f 'cursor-agent.*index.js -p' 2>/dev/null || true
  fi
  rm -f ~/pbsd/.git/index.lock
  setsid nohup env JOBS="${JOBS:-18}" GATE_JOBS="${GATE_JOBS:-6}" bash "$HOME/pbsd_driver.sh" >>~/pbsd_run.log 2>&1 &
  log "started pbsd_driver pid=$!"
}

exec 9>"$LOCK"
if ! flock -n 9; then
  log "another watchdog holds $LOCK — exiting"
  exit 0
fi

log "watchdog started pid=$$"

while true; do
  if ! pgrep -f '/home/odin/pbsd_driver.sh' >/dev/null 2>&1; then
    if pgrep -f 'python3.*pbsd.py' >/dev/null 2>&1; then
      log "driver shell gone but pbsd.py still running — waiting"
    else
      log "driver not running — restarting"
      start_driver
    fi
  fi
  push_if_needed
  sleep 120
done
