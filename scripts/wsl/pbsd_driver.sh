#!/bin/bash
# Reruns DeepSeek agent-port until stubbed work is drained (or rounds exhausted).
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
cd "$HOME/pbsd" || exit 1

JOBS="${JOBS:-48}"
PRO_JOBS="${PRO_JOBS:-24}"
SCOPE="${SCOPE:-}"
LOG="$HOME/pbsd_run.log"
MAX_ROUNDS=50

pending() {
  python3 - <<'PY'
import json
from pathlib import Path
p = Path("docs/migration/batch_progress.json")
if not p.is_file():
    print(0)
    raise SystemExit
data = json.loads(p.read_text(encoding="utf-8"))
n = sum(1 for e in data.get("entries", []) if e.get("status") in ("stubbed", "NEEDS-REVIEW", "pending", "PENDING"))
print(n)
PY
}

{
  echo "================================================================"
  echo "PBSD driver $(date -u +%FT%TZ)  jobs=$JOBS  pro_jobs=$PRO_JOBS  model=deepseek-v4-flash->pro"
  echo "================================================================"
} >>"$LOG"

for round in $(seq 1 "$MAX_ROUNDS"); do
  left=$(pending)
  echo "" >>"$LOG"
  echo "########## round $round — $left stubbed/needs-review — $(date -u +%FT%TZ) ##########" >>"$LOG"
  if [ "$left" = "0" ]; then
    echo "nothing pending" >>"$LOG"
    break
  fi
  args=(--jobs "$JOBS" --pro-jobs "$PRO_JOBS")
  if [ -n "$SCOPE" ]; then
    args+=(--scope "$SCOPE")
  fi
  python3 -u pbsd.py "${args[@]}" >>"$LOG" 2>&1
  rc=$?
  echo "round $round exited rc=$rc" >>"$LOG"
  git add -A >/dev/null 2>&1
  git commit -q -m "pbsd: round $round checkpoint" >/dev/null 2>&1 || true
  sleep 15
done

{
  echo ""
  echo "########## driver finished $(date -u +%FT%TZ) ##########"
} >>"$LOG"
python3 -u pbsd.py --status >>"$LOG" 2>&1
