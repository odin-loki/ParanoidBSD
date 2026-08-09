# Keep pbsd watchdog alive in WSL (survives driver death; relaunches if WSL is up).
$ErrorActionPreference = "Stop"
$log = "C:\Users\odinl\OneDrive\Desktop\Operating System\docs\migration\watchdog-keepalive.log"
function Log($msg) {
    $line = "$(Get-Date -Format 'yyyy-MM-ddTHH:mm:ssZ') $msg"
    Add-Content -Path $log -Value $line
}
try {
    $out = wsl -d Ubuntu -- bash -lc 'pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null 2>&1 || setsid nohup ~/pbsd_watchdog.sh >>~/pbsd_watchdog.log 2>&1 & echo started_or_running'
    Log "keepalive: $out"
} catch {
    Log "keepalive FAILED: $_"
    exit 1
}
