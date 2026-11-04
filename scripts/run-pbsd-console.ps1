# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# PBSD migration console — DeepSeek agent-port progress (no Cursor).
$ErrorActionPreference = "Continue"
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()
$Host.UI.RawUI.WindowTitle = "PBSD Migration"
$Repo = Split-Path $PSScriptRoot -Parent
. (Join-Path $PSScriptRoot 'load-pbsd-secrets.ps1')
Import-PbsdSecrets -Root $Repo
$RefreshSec = 10

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
    $r = Invoke-WslBash 'pkill -9 -f pbsd_watchdog 2>/dev/null; pkill -9 -f pbsd_driver 2>/dev/null; pkill -9 -f "python3.*pbsd.py" 2>/dev/null; rm -f /home/odin/pbsd_watchdog.lock; echo stopped' 30
    Write-Host "  $r" -ForegroundColor DarkGray
}

function Deploy-WslScripts {
    $wsl = '\\wsl$\Ubuntu\home\odin'
    $src = Join-Path $Repo 'scripts\wsl'
    foreach ($f in @('pbsd_watchdog.sh', 'pbsd_driver.sh', 'push_github.sh', 'load_secrets.sh')) {
        Copy-Item -Force (Join-Path $src $f) (Join-Path $wsl $f)
    }
    New-Item -ItemType Directory -Force -Path (Join-Path $wsl 'pbsd\tools\pbsd_agent') | Out-Null
    Copy-Item -Force (Join-Path $Repo 'pbsd.py') (Join-Path $wsl 'pbsd\pbsd.py')
    Copy-Item -Force (Join-Path $Repo 'tools\pbsd_secrets.py') (Join-Path $wsl 'pbsd\tools\pbsd_secrets.py')
    Copy-Item -Force (Join-Path $Repo 'tools\pbsd_agent_port.py') (Join-Path $wsl 'pbsd\tools\pbsd_agent_port.py')
    Copy-Item -Force -Recurse (Join-Path $Repo 'tools\pbsd_agent\*') (Join-Path $wsl 'pbsd\tools\pbsd_agent')
    $ok = Invoke-WslBash 'python3 -m py_compile ~/pbsd/pbsd.py ~/pbsd/tools/pbsd_secrets.py ~/pbsd/tools/pbsd_agent_port.py && echo ok' 45
    if ($ok -ne 'ok') {
        Write-Host '  WARNING: pbsd.py failed syntax check after deploy' -ForegroundColor Yellow
    }
    Invoke-WslBash 'sed -i "s/\r$//" ~/pbsd_watchdog.sh ~/pbsd_driver.sh ~/push_github.sh ~/load_secrets.sh ~/pbsd/pbsd.py ~/pbsd/tools/pbsd_secrets.py ~/pbsd/tools/pbsd_agent_port.py; find ~/pbsd/tools/pbsd_agent -name "*.py" -exec sed -i "s/\r$//" {} +; chmod +x ~/pbsd_watchdog.sh ~/pbsd_driver.sh ~/push_github.sh ~/load_secrets.sh' 45
}

function Start-PbsdWatchdog {
    Write-Host "Starting DeepSeek agent-port watchdog..." -ForegroundColor Green
    $r = Invoke-WslBash 'rm -f /home/odin/pbsd_watchdog.lock; setsid bash /home/odin/pbsd_watchdog.sh >>/home/odin/pbsd_watchdog.log 2>&1 & sleep 3; pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo started || echo failed' 45
    $key = Invoke-WslBash '. ~/load_secrets.sh 2>/dev/null; pbsd_load_secrets 2>/dev/null; if [ -n "$DEEPSEEK_API_KEY" ]; then echo ok; else echo missing; fi' 20
    Write-Host "  watchdog: $r" -ForegroundColor DarkGray
    if ($key -ne 'ok') {
        Write-Host "  WARNING: DEEPSEEK_API_KEY missing — put it in secrets/api-keys" -ForegroundColor Yellow
    } else {
        Write-Host "  DeepSeek key: OK" -ForegroundColor DarkGray
    }
}

function Get-PbsdStatus {
    $raw = Invoke-WslBash 'cd ~/pbsd && python3 pbsd.py --status 2>/dev/null' 90
    $s = @{ Converted = 0; Stubbed = 0; NeedsReview = 0; Total = 0; Raw = $raw }
    if ($raw -match 'converted:\s+(\d+)') { $s.Converted = [int]$Matches[1] }
    if ($raw -match 'stubbed:\s+(\d+)') { $s.Stubbed = [int]$Matches[1] }
    if ($raw -match 'NEEDS-REVIEW:\s+(\d+)') { $s.NeedsReview = [int]$Matches[1] }
    if ($raw -match 'entries=(\d+)') { $s.Total = [int]$Matches[1] }
    return $s
}

function Get-ProcessInfo {
    $wd = (Invoke-WslBash 'pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo on || echo off' 10).Trim()
    $dr = (Invoke-WslBash 'pgrep -f /home/odin/pbsd_driver.sh >/dev/null && echo on || echo off' 10).Trim()
    $py = (Invoke-WslBash 'pgrep -cf "python3.*pbsd.py" 2>/dev/null || echo 0' 10).Trim() -replace '\D', ''
    $memLine = (Invoke-WslBash 'free -m | sed -n 2p' 10).Trim()
    $mem = '?'
    if ($memLine -match 'Mem:\s+(\d+)\s+(\d+)') {
        $mem = "$($Matches[2])/$($Matches[1]) MB"
    }
    return @{
        Watchdog = if ($wd) { $wd } else { 'off' }
        Driver   = if ($dr) { $dr } else { 'off' }
        Workers  = [int]$py
        Mem      = $mem
    }
}

function Get-LogTail([int]$n = 12) {
    return Invoke-WslBash "tail -$n /home/odin/pbsd_run.log 2>/dev/null" 15
}

function Draw-Bar([double]$pct, [int]$width = 44) {
    $pct = [Math]::Max(0, [Math]::Min(100, $pct))
    $filled = [Math]::Round($width * $pct / 100)
    $empty = $width - $filled
    return '[' + ('#' * $filled) + ('-' * $empty) + (' ] {0:N1}%%' -f $pct)
}

function Show-Console($s, $proc, $logLines) {
    Clear-Host
    $done = $s.Converted
    $left = $s.Stubbed + $s.NeedsReview
    $total = if ($s.Total -gt 0) { $s.Total } else { $done + $left }
    $filePct = if ($total -gt 0) { 100.0 * $done / $total } else { 0 }

    Write-Host ""
    Write-Host "  PBSD C++23 Migration (DeepSeek)" -ForegroundColor Cyan
    Write-Host "  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  |  refresh ${RefreshSec}s  |  Ctrl+C closes viewer only" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  Converted  $done / $total" -ForegroundColor White
    Write-Host "  $(Draw-Bar $filePct)" -ForegroundColor Green
    Write-Host ""
    Write-Host "  Metrics" -ForegroundColor Cyan
    Write-Host "    stubbed $($s.Stubbed)    needs-review $($s.NeedsReview)    remaining $left"
    Write-Host ""
    Write-Host "  Processes" -ForegroundColor Cyan
    Write-Host "    watchdog $($proc.Watchdog)   driver $($proc.Driver)   python $($proc.Workers)   RAM $($proc.Mem)"
    Write-Host ""
    Write-Host "  Recent log" -ForegroundColor Cyan
    foreach ($ln in $logLines) {
        if (-not $ln) { continue }
        $t = $ln -replace '[^\x09\x0A\x0D\x20-\x7E]', '?'
        if ($t -match 'converted') { Write-Host "    $t" -ForegroundColor Green }
        elseif ($t -match 'NEEDS-REVIEW|WARNING|failed|error|MISSING') { Write-Host "    $t" -ForegroundColor Yellow }
        else { Write-Host "    $t" -ForegroundColor Gray }
    }
    Write-Host ""
}

$arg = $args | Select-Object -First 1
if ($arg -eq 'stop') {
    Stop-PbsdServices
    exit 0
}

$mutex = New-Object System.Threading.Mutex($false, 'Global\PbsdMigrationConsole')
if (-not $mutex.WaitOne(0, $false)) {
    Write-Host 'PBSD console is already open.' -ForegroundColor Yellow
    exit 0
}

Clear-Host
Write-Host "PBSD Migration Console (DeepSeek)" -ForegroundColor Cyan
Write-Host ""

$alreadyRunning = @(
    (Invoke-WslBash 'pgrep -f /home/odin/pbsd_watchdog.sh >/dev/null && echo on || echo off' 10).Trim()
    (Invoke-WslBash 'pgrep -f /home/odin/pbsd_driver.sh >/dev/null && echo on || echo off' 10).Trim()
) -contains 'on'

if ($arg -eq 'attach' -or ($arg -ne 'restart' -and $alreadyRunning)) {
    if ($alreadyRunning) {
        Write-Host "Attached to running driver (no restart)." -ForegroundColor Cyan
    } else {
        Write-Host "Nothing running — starting driver..." -ForegroundColor Yellow
        Deploy-WslScripts
        Start-PbsdWatchdog
        Start-Sleep -Seconds 4
    }
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
        $logLines = @(Get-LogTail | Out-String).Trim() -split "`n"
        Show-Console $s $proc $logLines

        if (($s.Stubbed + $s.NeedsReview) -eq 0 -and $proc.Driver -eq 'off') {
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
