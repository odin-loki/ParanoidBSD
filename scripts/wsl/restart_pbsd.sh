#!/bin/bash
set -euo pipefail
ROOT="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System"
for f in pbsd_watchdog.sh pbsd_driver.sh push_github.sh sync_cursor_auth.sh; do
  cp -f "$ROOT/scripts/wsl/$f" ~/"$f"
  sed -i 's/\r$//' ~/"$f"
  chmod +x ~/"$f"
done
cp -f "$ROOT/pbsd.py" ~/pbsd/pbsd.py
sed -i 's/\r$//' ~/pbsd/pbsd.py
python3 -m py_compile ~/pbsd/pbsd.py
pkill -9 -f pbsd_watchdog 2>/dev/null || true
pkill -9 -f pbsd_driver 2>/dev/null || true
pkill -9 -f 'python3.*pbsd.py' 2>/dev/null || true
pkill -9 -f 'cursor-agent.*index.js -p' 2>/dev/null || true
rm -f ~/pbsd_watchdog.lock
sleep 2
bash ~/sync_cursor_auth.sh 2>/dev/null || true
setsid bash ~/pbsd_watchdog.sh >>~/pbsd_watchdog.log 2>&1 &
sleep 6
echo "=== status ==="
cd ~/pbsd && python3 pbsd.py --status | head -10
echo "=== processes ==="
pgrep -af pbsd_watchdog || echo watchdog:off
pgrep -af pbsd_driver || echo driver:off
echo "=== watchdog log ==="
tail -3 ~/pbsd_watchdog.log
echo "=== run log ==="
tail -3 ~/pbsd_run.log
