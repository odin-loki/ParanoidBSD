# PBSD migration console — progress bars and metrics (no Write-Progress overlap).
$ErrorActionPreference = "Continue"
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
$Host.UI.RawUI.WindowTitle = "PBSD Migration"
$Repo = Split-Path $PSScriptRoot -Parent
$RefreshSec = 10
$TotalFiles = 4497
$TotalLines = 2875339

function Invoke-WslBash([string]$Script, [int]$TimeoutSec = 120) {
    $raw = & wsl.exe -d Ubuntu -- timeout $TimeoutSec bash -c $Script 2>&1
    $lines = @()
    foreach ($item in $raw) {
        if ($item -is [System.Management.Automation.ErrorRecord]) { continue }
        $lines += [string]$item
    }
    return ($lines -join "`n").Trim()
}

function Stop-PbsdServices {
    Write-Host "Stopping PBSD processes..." -ForegroundColor Yellow
    $r = Invoke-WslBash 'pkill -9 -f pbsd_watchdog 2>/dev/null; pkill -9 -f pbsd_driver 2>/dev/null; pkill -9 -f "python3.*pbsd.py" 2>/dev/null; pkill -9 -f cursor-agent 2>/dev/null; rm -f /home/odin/pbsd_watchdog.lock; echo stopped' 30
    Write-Host "  $r" -ForegroundColor DarkGray
}

function Deploy-WslScripts {
    $cmds = @(
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/pbsd_watchdog.sh > /home/odin/pbsd_watchdog.sh'
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/pbsd_driver.sh > /home/odin/pbsd_driver.sh'
        'tr -d "\r" < /mnt/c/Users/odinl/AppData/Local/Temp/pbsd_run/push_github.sh > /home/odin/push_github.sh'
        'tr -d "\r" < "/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/pbsd.py" > /home/odin/pbsd/pbsd.py'
        'chmod +x /home/odin/pbsd_watchdog.sh /home/odin/pbsd_driver.sh /home/odin/push_github.sh'
    )
    foreach ($c in $cmds) { Invoke-WslBash $c 30 }
}

function Start-PbsdWatchdog {
    Write-Host "Starting watchdog + refreshing auth..." -ForegroundColor Green
    $r = Invoke-WslBash 'bash ~/sync_cursor_auth.sh 2>/dev/null; rm -f /home/odin/pbsd_watchdog.lock; setsid bash /home/odin/pbsd_watchdog.sh >>/home/odin/pbsd_watchdog.log 2>&1 & sleep 3; pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo started || echo failed' 45
    $auth = Invoke-WslBash 'unset CURSOR_API_KEY; timeout 45 cursor-agent -p "Reply READY" --model composer-2.5 --output-format text 2>&1 | tail -1' 60
    Write-Host "  watchdog: $r" -ForegroundColor DarkGray
    if ($auth -notmatch 'READY') {
        Write-Host "  WARNING: cursor-agent auth check failed — run: wsl -d Ubuntu, then cursor-agent login" -ForegroundColor Yellow
    } else {
        Write-Host "  cursor-agent auth: OK" -ForegroundColor DarkGray
    }
}

function Get-PbsdStatus {
    $raw = Invoke-WslBash 'cd ~/pbsd && python3 pbsd.py --status 2>/dev/null | head -14' 90
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
    $wd = (Invoke-WslBash 'pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo on || echo off' 10).Trim()
    $dr = (Invoke-WslBash 'pgrep -f /home/odin/pbsd_driver.sh >/dev/null && echo on || echo off' 10).Trim()
    $ag = (Invoke-WslBash 'pgrep -cf cursor-agent 2>/dev/null || echo 0' 10).Trim() -replace '\D', ''
    $memLine = (Invoke-WslBash 'free -m | sed -n 2p' 10).Trim()
    $mem = '?'
    if ($memLine -match 'Mem:\s+(\d+)\s+(\d+)') {
        $mem = "$($Matches[2])/$($Matches[1]) MB"
    }
    return @{
        Watchdog = if ($wd) { $wd } else { 'off' }
        Driver   = if ($dr) { $dr } else { 'off' }
        Agents   = [int]$ag
        Mem      = $mem
    }
}

function Get-LogTail([int]$n = 10) {
    return Invoke-WslBash "tail -$n /home/odin/pbsd_run.log 2>/dev/null" 15
}

function Get-BatchProgress {
    $line = Invoke-WslBash 'grep -E "\[[0-9]+/[0-9]+\]" /home/odin/pbsd_run.log 2>/dev/null | tail -1' 15
    $done = 0; $total = 0; $eta = 0.0
    if ($line -match '\[(\d+)/(\d+)\]') {
        $done = [int]$Matches[1]
        $total = [int]$Matches[2]
    }
    if ($line -match '~([\d.]+)h left') { $eta = [double]$Matches[1] }
    return @{ Done = $done; Total = $total; EtaHours = $eta }
}

function Draw-Bar([double]$pct, [int]$width = 44) {
    $pct = [Math]::Max(0, [Math]::Min(100, $pct))
    $filled = [Math]::Round($width * $pct / 100)
    $empty = $width - $filled
    return '[' + ('#' * $filled) + ('-' * $empty) + "] $([math]::Round($pct,1))%"
}

function Show-Console($s, $proc, $batch, $logLines) {
    Clear-Host
    $filePct = if ($s.TotalFiles -gt 0) { 100.0 * $s.Verified / $s.TotalFiles } else { 0 }
    $linePct = if ($s.TotalLines -gt 0) { 100.0 * $s.Lines / $s.TotalLines } else { 0 }

    Write-Host ""
    Write-Host "  PBSD C++23 Migration" -ForegroundColor Cyan
    Write-Host "  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  |  refresh ${RefreshSec}s  |  Ctrl+C closes viewer only" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  Files  $($s.Verified) / $($s.TotalFiles)" -ForegroundColor White
    Write-Host "  $(Draw-Bar $filePct)" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Lines  $($s.Lines.ToString('N0')) / $($s.TotalLines.ToString('N0'))  ($([math]::Round($linePct,1))%)" -ForegroundColor White
    Write-Host "  $(Draw-Bar $linePct)" -ForegroundColor DarkGreen
    Write-Host ""
    Write-Host "  Metrics" -ForegroundColor Cyan
    Write-Host "    pending    $($s.Pending)    deferred $($s.Deferred)    need-you $($s.NeedYou)    skipped $($s.Skipped)"
    Write-Host "    pass rate  $($s.PassRate)%"
    Write-Host ""
    Write-Host "  Processes" -ForegroundColor Cyan
    Write-Host "    watchdog $($proc.Watchdog)   driver $($proc.Driver)   agents $($proc.Agents)   RAM $($proc.Mem)"
    if ($batch.Total -gt 0) {
        $bp = [math]::Round(100.0 * $batch.Done / $batch.Total, 1)
        Write-Host "    round      [$($batch.Done)/$($batch.Total)]  ~$($batch.EtaHours)h left  ($bp%)" -ForegroundColor DarkCyan
    }
    Write-Host ""
    Write-Host "  Recent log" -ForegroundColor Cyan
    foreach ($ln in $logLines) {
        if (-not $ln) { continue }
        $t = $ln -replace '[^\x09\x0A\x0D\x20-\x7E]', '?'
        if ($t -match 'VERIFIED') { Write-Host "    $t" -ForegroundColor Green }
        elseif ($t -match 'WARNING|failed|error|auth') { Write-Host "    $t" -ForegroundColor Yellow }
        else { Write-Host "    $t" -ForegroundColor Gray }
    }
    Write-Host ""
}

$arg = $args | Select-Object -First 1
if ($arg -eq 'stop') {
    Stop-PbsdServices
    exit 0
}

Clear-Host
Write-Host "PBSD Migration Console" -ForegroundColor Cyan
Write-Host ""

if ($arg -eq 'attach') {
    Write-Host "Attached (no restart)." -ForegroundColor Cyan
} else {
    Stop-PbsdServices
    Deploy-WslScripts
    Start-PbsdWatchdog
    Write-Host "Migration running. Metrics below." -ForegroundColor Green
    Start-Sleep -Seconds 4
}

try {
    while ($true) {
        $s = Get-PbsdStatus
        $proc = Get-ProcessInfo
        $batch = Get-BatchProgress
        $logLines = @(Get-LogTail | Out-String).Trim() -split "`n"

        Show-Console $s $proc $batch $logLines

        if ($s.Pending -eq 0 -and $s.Deferred -eq 0 -and $proc.Driver -eq 'off') {
            Write-Host "  Queue empty." -ForegroundColor Green
            break
        }

        Start-Sleep -Seconds $RefreshSec
    }
} catch {
    Write-Host "Monitor error: $_" -ForegroundColor Red
}

Write-Host ""
Read-Host "Press Enter to close"
