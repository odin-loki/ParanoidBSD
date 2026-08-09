# PBSD migration console — progress bar, metrics, starts watchdog, stays open.
$ErrorActionPreference = "Continue"
$Host.UI.RawUI.WindowTitle = "PBSD Migration"
$Repo = Split-Path $PSScriptRoot -Parent
$RefreshSec = 10
$TotalFiles = 4497
$TotalLines = 2875339

function Wsl([string]$Script) {
    $out = wsl -d Ubuntu -- bash -lc $Script 2>&1
    if ($out -is [System.Array]) { return ($out | Out-String).Trim() }
    return [string]$out
}

function Stop-PbsdServices {
    Write-Host "Stopping any running PBSD processes..." -ForegroundColor Yellow
    Wsl 'pkill -9 -f pbsd_watchdog.sh 2>/dev/null; pkill -9 -f pbsd_driver.sh 2>/dev/null; pkill -9 -f "python3 -u pbsd.py" 2>/dev/null; pkill -9 -f "cursor-agent.*index.js" 2>/dev/null; sleep 1; echo stopped'
}

function Deploy-WslScripts {
    $cmds = @(
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/pbsd_watchdog.sh > /home/odin/pbsd_watchdog.sh'
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/pbsd_driver.sh > /home/odin/pbsd_driver.sh'
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/push_github.sh > /home/odin/push_github.sh'
        'tr -d "\r" < "/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd.py" > /home/odin/pbsd/pbsd.py'
        'chmod +x /home/odin/pbsd_watchdog.sh /home/odin/pbsd_driver.sh /home/odin/push_github.sh'
    )
    foreach ($c in $cmds) { Wsl $c }
    return 'deployed'
}

function Start-PbsdWatchdog {
    $running = Wsl 'pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null 2>&1 && echo yes || echo no'
    if ($running -match 'yes') { return }
    Write-Host "Starting migration watchdog..." -ForegroundColor Green
    Wsl 'setsid bash /home/odin/pbsd_watchdog.sh >>/home/odin/pbsd_watchdog.log 2>&1 & sleep 2; pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo started || echo failed'
}

function Get-PbsdStatus {
    $raw = Wsl 'cd ~/pbsd && python3 pbsd.py --status 2>/dev/null'
    $s = @{
        Verified = 0; TotalFiles = $TotalFiles
        Lines = 0; TotalLines = $TotalLines
        Pending = 0; Deferred = 0; PassRate = 0.0
        NeedYou = 0; Skipped = 0
    }
    if ($raw -match 'verified\s+(\d+)\s*/\s*(\d+)\s+files\s+([\d,]+)\s*/\s*([\d,]+)\s+lines') {
        $s.Verified = [int]$Matches[1]
        $s.TotalFiles = [int]$Matches[2]
        $s.Lines = [int]($Matches[3] -replace ',', '')
        $s.TotalLines = [int]($Matches[4] -replace ',', '')
    }
    if ($raw -match 'pending\s+(\d+)') { $s.Pending = [int]$Matches[1] }
    if ($raw -match 'deferred\s+(\d+)') { $s.Deferred = [int]$Matches[1] }
    if ($raw -match 'need you\s+(\d+)') { $s.NeedYou = [int]$Matches[1] }
    if ($raw -match 'skipped\s+(\d+)') { $s.Skipped = [int]$Matches[1] }
    if ($raw -match 'pass rate\s+([\d.]+)%') { $s.PassRate = [double]$Matches[1] }
    return $s
}

function Get-ProcessInfo {
    $raw = Wsl 'printf "watchdog=%s driver=%s agents=%s mem=%s\n" "$(pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo on || echo off)" "$(pgrep -f /home/odin/pbsd_driver.sh >/dev/null && echo on || echo off)" "$(pgrep -cf cursor-agent 2>/dev/null || echo 0)" "$(free -h | awk "/^Mem:/ {print \$3\"/\"\$2}")"'
    $info = @{ Watchdog = '?'; Driver = '?'; Agents = 0; Mem = '?' }
    if ($raw -match 'watchdog=(\w+)') { $info.Watchdog = $Matches[1] }
    if ($raw -match 'driver=(\w+)') { $info.Driver = $Matches[1] }
    if ($raw -match 'agents=(\d+)') { $info.Agents = [int]$Matches[1] }
    if ($raw -match 'mem=([^\s]+)') { $info.Mem = $Matches[1] }
    return $info
}

function Get-LogTail([int]$n = 12) {
    return Wsl "tail -$n /home/odin/pbsd_run.log 2>/dev/null"
}

function Get-BatchProgress {
    $line = Wsl 'grep -E "\[[0-9]+/[0-9]+\]" /home/odin/pbsd_run.log 2>/dev/null | tail -1'
    if ($line -match '\[(\d+)/(\d+)\].*?~([\d.]+)h left') {
        return @{ Done = [int]$Matches[1]; Total = [int]$Matches[2]; EtaHours = [double]$Matches[3]; Line = $line.Trim() }
    }
    if ($line -match '\[(\d+)/(\d+)\]') {
        return @{ Done = [int]$Matches[1]; Total = [int]$Matches[2]; EtaHours = 0; Line = $line.Trim() }
    }
    return @{ Done = 0; Total = 0; EtaHours = 0; Line = '' }
}

function Draw-Bar([int]$pct, [int]$width = 40) {
    $pct = [Math]::Max(0, [Math]::Min(100, $pct))
    $filled = [Math]::Round($width * $pct / 100)
    $empty = $width - $filled
    return ('[' + ('#' * $filled) + ('-' * $empty) + "] $pct%")
}

function Show-Console($s, $proc, $batch, $log) {
    Clear-Host
    $filePct = if ($s.TotalFiles -gt 0) { [math]::Round(100 * $s.Verified / $s.TotalFiles, 2) } else { 0 }
    $linePct = if ($s.TotalLines -gt 0) { [math]::Round(100 * $s.Lines / $s.TotalLines, 2) } else { 0 }

    Write-Host "  PBSD C++23 Migration" -ForegroundColor Cyan
    Write-Host "  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  |  refresh every ${RefreshSec}s  |  Ctrl+C closes viewer (driver keeps running)" -ForegroundColor DarkGray
    Write-Host ""

    Write-Host "  Files   $($s.Verified) / $($s.TotalFiles)" -ForegroundColor White
    Write-Host "  $(Draw-Bar $filePct)" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Lines   $($s.Lines.ToString('N0')) / $($s.TotalLines.ToString('N0'))  ($linePct%)" -ForegroundColor White
    Write-Host "  $(Draw-Bar $linePct)" -ForegroundColor DarkGreen
    Write-Host ""

    Write-Host "  Metrics" -ForegroundColor Cyan
    Write-Host "    pending      $($s.Pending)"
    Write-Host "    deferred     $($s.Deferred)"
    Write-Host "    need you     $($s.NeedYou)"
    Write-Host "    skipped      $($s.Skipped)"
    Write-Host "    pass rate    $($s.PassRate)%"
    Write-Host ""
    Write-Host "  Processes" -ForegroundColor Cyan
    Write-Host "    watchdog     $($proc.Watchdog)    driver $($proc.Driver)    agents $($proc.Agents)    RAM $($proc.Mem)"
    if ($batch.Total -gt 0) {
        $batchPct = [math]::Round(100 * $batch.Done / $batch.Total, 1)
        Write-Host "    this round   [$batch.Done/$batch.Total]  ~$($batch.EtaHours)h left  ($batchPct%)" -ForegroundColor DarkCyan
    }
    Write-Host ""
    Write-Host "  Recent log" -ForegroundColor Cyan
    foreach ($ln in ($log -split "`n")) {
        if ($ln -match 'VERIFIED') { Write-Host "    $ln" -ForegroundColor Green }
        elseif ($ln -match 'WARNING|failed|error') { Write-Host "    $ln" -ForegroundColor Yellow }
        else { Write-Host "    $ln" -ForegroundColor Gray }
    }
}

Clear-Host
Write-Host "PBSD Migration Console" -ForegroundColor Cyan
Write-Host ""

$arg = $args | Select-Object -First 1
if ($arg -eq 'stop') {
    Stop-PbsdServices
    exit 0
}

if ($arg -eq 'attach') {
    Write-Host "Attached to running migration (no restart)." -ForegroundColor Cyan
    Start-Sleep -Seconds 1
} else {
    Stop-PbsdServices
    Deploy-WslScripts
    Start-PbsdWatchdog
    Write-Host ""
    Write-Host "Migration started. This window stays open with live metrics." -ForegroundColor Green
    Start-Sleep -Seconds 3
}

try {
    while ($true) {
        $s = Get-PbsdStatus
        $proc = Get-ProcessInfo
        $batch = Get-BatchProgress
        $log = Get-LogTail

        $filePct = if ($s.TotalFiles -gt 0) { 100 * $s.Verified / $s.TotalFiles } else { 0 }
        Write-Progress -Activity "PBSD files" -Status "$($s.Verified) / $($s.TotalFiles) verified" -PercentComplete $filePct
        $linePct = if ($s.TotalLines -gt 0) { 100 * $s.Lines / $s.TotalLines } else { 0 }
        Write-Progress -Activity "PBSD lines" -Status "$($s.Lines.ToString('N0')) / $($s.TotalLines.ToString('N0'))" -PercentComplete $linePct -Id 2

        Show-Console $s $proc $batch $log

        if ($s.Pending -eq 0 -and $s.Deferred -eq 0 -and $proc.Driver -eq 'off') {
            Write-Host ""
            Write-Host "  Migration queue empty." -ForegroundColor Green
            break
        }

        Start-Sleep -Seconds $RefreshSec
    }
} finally {
    Write-Progress -Activity "PBSD files" -Completed
    Write-Progress -Activity "PBSD lines" -Completed -Id 2
}

Write-Host ""
Read-Host "Press Enter to close"
