# DeepSeek agent-port runner (pbsd.py).
# Defaults: Flash×48 + Pro×24, reasoning_effort=max, failures saved.
# Usage (from repo root, PowerShell):
#   .\scripts\run_agent_port_deepseek.ps1
#   .\scripts\run_agent_port_deepseek.ps1 -Scope "bin,usr.bin" -Limit 50
#   .\scripts\run_agent_port_deepseek.ps1 -DryRun

param(
    [string]$Scope = "",
    [Nullable[int]]$Limit = $null,
    [int]$Skip = 0,
    [int]$Jobs = 48,
    [int]$ProJobs = 24,
    [int]$MaxRetries = 2,
    [double]$FileTimeout = 600,
    [switch]$DryRun,
    [switch]$SelfTest,
    [switch]$Status,
    [string[]]$File = @()
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

if (-not (Test-Path "secrets\api-keys")) {
    Write-Error "Missing secrets\api-keys — copy from secrets\api-keys.example and set DEEPSEEK_API_KEY"
}

$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command py -ErrorAction SilentlyContinue }
if (-not $py) { Write-Error "python not on PATH" }

$argsList = @("pbsd.py")
if ($Status) {
    $argsList += "--status"
    & $py.Source @argsList
    exit $LASTEXITCODE
}
if ($SelfTest) {
    $argsList += "--self-test"
    & $py.Source @argsList
    exit $LASTEXITCODE
}

if ($DryRun) { $argsList += "--dry-run" }
if ($Scope) { $argsList += @("--scope", $Scope) }
if ($null -ne $Limit) { $argsList += @("--limit", "$Limit") }
if ($Skip -gt 0) { $argsList += @("--skip", "$Skip") }
$argsList += @("--jobs", "$Jobs", "--pro-jobs", "$ProJobs", "--max-retries", "$MaxRetries", "--file-timeout", "$FileTimeout")
foreach ($f in $File) { $argsList += @("--file", $f) }

Write-Host "Running: $($py.Source) $($argsList -join ' ')"
& $py.Source @argsList
exit $LASTEXITCODE
