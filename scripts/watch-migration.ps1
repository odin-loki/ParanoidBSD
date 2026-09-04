# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Live PBSD migration console — DeepSeek agent-port (driver keeps running).
$Host.UI.RawUI.WindowTitle = "PBSD Migration Monitor"
$refreshSec = 15

function Write-Header {
    param([string]$Title)
    Write-Host ""
    Write-Host $Title -ForegroundColor Cyan
}

while ($true) {
    try {
        Clear-Host
        Write-Header "=== PBSD Migration Monitor  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') ==="

        $status = wsl -d Ubuntu -- bash -lc 'cd ~/pbsd && python3 pbsd.py --status 2>/dev/null | head -20'
        if ($status) { Write-Host $status }

        $procs = wsl -d Ubuntu -- bash -lc @'
printf "watchdog: "; pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo yes || echo no
printf "driver:   "; pgrep -f /home/odin/pbsd_driver.sh >/dev/null && echo yes || echo no
printf "python:   "; pgrep -cf "python3.*pbsd.py" 2>/dev/null || echo 0
printf "memory:   "; free -h | awk "/^Mem:/ {print \$3 \" used / \" \$2 \" total\"}"
'@
        Write-Host ""
        Write-Host $procs -ForegroundColor DarkCyan

        Write-Header "--- log (last 25 lines) ---"
        wsl -d Ubuntu -- tail -25 /home/odin/pbsd_run.log 2>$null

        Write-Host ""
        Write-Host "Refreshing every ${refreshSec}s. Close this window to stop watching (driver keeps running)." -ForegroundColor DarkGray
        Start-Sleep -Seconds $refreshSec
    } catch {
        Write-Host "Monitor error: $_" -ForegroundColor Red
        Start-Sleep -Seconds $refreshSec
    }
}
