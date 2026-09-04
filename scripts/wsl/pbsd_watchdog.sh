#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Outer supervisor: keeps pbsd_driver alive and pushes periodically.
export PATH="$HOME/.local/bin:$PATH"
if [ -f "$HOME/load_secrets.sh" ]; then
  # shellcheck source=/dev/null
  . "$HOME/load_secrets.sh"
  pbsd_load_secrets
elif [ -f "/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/scripts/wsl/load_secrets.sh" ]; then
  # shellcheck source=/dev/null
  . "/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/scripts/wsl/load_secrets.sh"
  pbsd_load_secrets
fi
LOG="$HOME/pbsd_watchdog.log"
PUSH_INTERVAL="${PUSH_INTERVAL:-1800}"
LOCK="$HOME/pbsd_watchdog.lock"
WIN="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System"
last_push=0

log() { echo "$(date -u +%FT%TZ) $*" | tee -a "$LOG"; }

push_if_needed() {
  now=$(date +%s)
  if [ $((now - last_push)) -lt "$PUSH_INTERVAL" ]; then return 0; fi
  log "running github push"
  bash "$HOME/push_github.sh" >>"$LOG" 2>&1 || log "github push failed"
  last_push=$now
}

sync_file() {
  local src="$1" dest="$2"
  local tmp
  [ -f "$src" ] || return 0
  mkdir -p "$(dirname "$dest")"
  tmp="$(mktemp)"
  cp -f "$src" "$tmp"
  sed -i 's/\r$//' "$tmp"
  mv "$tmp" "$dest"
}

start_driver() {
  sync_file "$WIN/pbsd.py" "$HOME/pbsd/pbsd.py"
  sync_file "$WIN/tools/pbsd_secrets.py" "$HOME/pbsd/tools/pbsd_secrets.py"
  sync_file "$WIN/tools/pbsd_agent_port.py" "$HOME/pbsd/tools/pbsd_agent_port.py"
  if [ -d "$WIN/tools/pbsd_agent" ]; then
    mkdir -p "$HOME/pbsd/tools/pbsd_agent"
    cp -rf "$WIN/tools/pbsd_agent/." "$HOME/pbsd/tools/pbsd_agent/"
    find "$HOME/pbsd/tools/pbsd_agent" -type f -name '*.py' -exec sed -i 's/\r$//' {} +
  fi
  sync_file "$WIN/scripts/wsl/load_secrets.sh" "$HOME/load_secrets.sh"
  chmod +x "$HOME/load_secrets.sh" 2>/dev/null || true
  sync_file "$WIN/scripts/wsl/pbsd_driver.sh" "$HOME/pbsd_driver.sh"
  chmod +x "$HOME/pbsd_driver.sh" 2>/dev/null || true

  rm -f ~/pbsd/.git/index.lock
  setsid nohup env JOBS="${JOBS:-48}" PRO_JOBS="${PRO_JOBS:-24}" SCOPE="${SCOPE:-}" \
    bash "$HOME/pbsd_driver.sh" >>~/pbsd_run.log 2>&1 &
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
  sleep 30
done
