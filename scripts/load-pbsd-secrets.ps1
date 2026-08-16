# Dot-source, then Import-PbsdSecrets. Loads secrets/api-keys into the process env.
function Import-PbsdSecrets {
    param(
        [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
    )
    $files = @()
    if ($env:PBSD_SECRETS) { $files += $env:PBSD_SECRETS }
    $files += (Join-Path $Root 'secrets\api-keys')
    $files += (Join-Path $Root 'secrets\api-keys.env')
    foreach ($f in $files) {
        if (-not $f -or -not (Test-Path -LiteralPath $f)) { continue }
        Get-Content -LiteralPath $f | ForEach-Object {
            $line = $_.Trim()
            if (-not $line -or $line.StartsWith('#')) { return }
            if ($line.StartsWith('export ')) { $line = $line.Substring(7).Trim() }
            $eq = $line.IndexOf('=')
            if ($eq -lt 1) { return }
            $k = $line.Substring(0, $eq).Trim()
            $v = $line.Substring($eq + 1).Trim()
            if ($v.Length -ge 2 -and (($v.StartsWith('"') -and $v.EndsWith('"')) -or ($v.StartsWith("'") -and $v.EndsWith("'")))) {
                $v = $v.Substring(1, $v.Length - 2)
            }
            if (-not $k -or -not $v) { return }
            $existing = [Environment]::GetEnvironmentVariable($k, 'Process')
            if (-not $existing) {
                Set-Item -Path "Env:$k" -Value $v
            }
        }
        return
    }
}
