#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
set -euo pipefail
ROOT="/mnt/c/Users/odinl/OneDrive/Desktop/Operating System"
for f in pbsd_watchdog.sh pbsd_driver.sh push_github.sh load_secrets.sh; do
  cp -f "$ROOT/scripts/wsl/$f" ~/"$f"
  sed -i 's/\r$//' ~/"$f"
  chmod +x ~/"$f"
done
mkdir -p ~/pbsd/tools/pbsd_agent
cp -f "$ROOT/pbsd.py" ~/pbsd/pbsd.py
sed -i 's/\r$//' ~/pbsd/pbsd.py
cp -f "$ROOT/tools/pbsd_secrets.py" ~/pbsd/tools/pbsd_secrets.py
cp -f "$ROOT/tools/pbsd_agent_port.py" ~/pbsd/tools/pbsd_agent_port.py
cp -rf "$ROOT/tools/pbsd_agent/." ~/pbsd/tools/pbsd_agent/
sed -i 's/\r$//' ~/pbsd/tools/pbsd_secrets.py ~/pbsd/tools/pbsd_agent_port.py
find ~/pbsd/tools/pbsd_agent -type f -name '*.py' -exec sed -i 's/\r$//' {} +
python3 -m py_compile ~/pbsd/pbsd.py ~/pbsd/tools/pbsd_secrets.py ~/pbsd/tools/pbsd_agent_port.py
pkill -f pbsd_watchdog 2>/dev/null || true
pkill -f pbsd_driver 2>/dev/null || true
pkill -f 'python3.*pbsd.py' 2>/dev/null || true
sleep 5
pkill -9 -f 'python3.*pbsd.py' 2>/dev/null || true
rm -f ~/pbsd_watchdog.lock
setsid bash ~/pbsd_watchdog.sh >>~/pbsd_watchdog.log 2>&1 &
sleep 6
echo "=== status ==="
cd ~/pbsd && python3 pbsd.py --status | head -20
echo "=== processes ==="
pgrep -af pbsd_watchdog || echo watchdog:off
pgrep -af pbsd_driver || echo driver:off
echo "=== deepseek key ==="
if [ -n "${DEEPSEEK_API_KEY:-}" ]; then echo set; else
  # shellcheck source=/dev/null
  . ~/load_secrets.sh 2>/dev/null || true
  pbsd_load_secrets 2>/dev/null || true
  if [ -n "${DEEPSEEK_API_KEY:-}" ]; then echo set; else echo MISSING; fi
fi
echo "=== run log ==="
tail -5 ~/pbsd_run.log 2>/dev/null || echo "(no log yet)"
