#!/bin/bash
# Reruns pbsd.py until nothing is pending. Progress is committed per batch.
export PATH="$HOME/.local/bin:$PATH"
unset CURSOR_API_KEY
cd "$HOME/pbsd" || exit 1

JOBS="${JOBS:-14}"
GATE_JOBS="${GATE_JOBS:-4}"
LOG="$HOME/pbsd_run.log"
MAX_ROUNDS=50

pending() {
  python3 - <<'PY'
import csv
rows = list(csv.DictReader(open("docs/migration/inventory.csv", encoding="utf-8")))
print(sum(1 for r in rows if r["status"] in ("PENDING", "REJECTED")))
PY
}

{
  echo "================================================================"
  echo "PBSD driver $(date -u +%FT%TZ)  jobs=$JOBS  gate_jobs=$GATE_JOBS  model=composer-2.5"
  echo "================================================================"
} >>"$LOG"

for round in $(seq 1 "$MAX_ROUNDS"); do
  left=$(pending)
  echo "" >>"$LOG"
  echo "########## round $round — $left pending — $(date -u +%FT%TZ) ##########" >>"$LOG"
  if [ "$left" = "0" ]; then
    echo "nothing pending; deferred pass" >>"$LOG"
    python3 -u pbsd.py --deferred --jobs "$JOBS" --gate-jobs "$GATE_JOBS" >>"$LOG" 2>&1
    break
  fi
  if [ "$round" = "1" ] && [ -z "${SKIP_MECH:-}" ]; then
    python3 -u pbsd.py --mechanical-only --jobs 64 >>"$LOG" 2>&1
  fi
  python3 -u pbsd.py --jobs "$JOBS" --gate-jobs "$GATE_JOBS" --no-mechanical >>"$LOG" 2>&1
  rc=$?
  echo "round $round exited rc=$rc" >>"$LOG"
  git add -A >/dev/null 2>&1
  git commit -q -m "pbsd: round $round checkpoint" >/dev/null 2>&1 || true
  sleep 30
done

{
  echo ""
  echo "########## driver finished $(date -u +%FT%TZ) ##########"
} >>"$LOG"
python3 -u pbsd.py --status >>"$LOG" 2>&1
