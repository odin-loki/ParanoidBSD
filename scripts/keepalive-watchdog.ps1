# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Keep pbsd watchdog alive in WSL (survives driver death; relaunches if WSL is up).
$ErrorActionPreference = "Stop"
$log = "C:\Users\odinl\OneDrive\Desktop\Operating System\docs\migration\watchdog-keepalive.log"
function Log($msg) {
    $line = "$(Get-Date -Format 'yyyy-MM-ddTHH:mm:ssZ') $msg"
    Add-Content -Path $log -Value $line
    Write-Output $line
}
try {
    $out = wsl -d Ubuntu -- bash -lc @'
python3 -c "
from pathlib import Path
tmp = Path('/mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run')
home = Path('/home/odin')
for n in ('pbsd_watchdog.sh', 'pbsd_driver.sh'):
    p = home / n
    p.write_text(tmp.joinpath(n).read_text().replace('\r\n','\n').replace('\r','\n'))
    p.chmod(0o755)
"
pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null 2>&1 || setsid bash /home/odin/pbsd_watchdog.sh >>/home/odin/pbsd_watchdog.log 2>&1 &
echo started_or_running
'@
    Log "keepalive: $out"
} catch {
    Log "keepalive FAILED: $_"
    exit 1
}
