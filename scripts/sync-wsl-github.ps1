# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
# Sync WSL ~/pbsd (wslclone remote) -> Windows working copy -> GitHub.
$Repo = "C:\Users\odinl\OneDrive\Desktop\Operating System"
Set-Location $Repo
$log = Join-Path $Repo "docs\migration\sync-wsl-github.log"
function Log($msg) {
    $line = "$(Get-Date -Format 'yyyy-MM-ddTHH:mm:ssZ') $msg"
    Add-Content -Path $log -Value $line
    Write-Output $line
}
Log "sync start"
& git fetch wslclone main | ForEach-Object { Log $_ }
if ($LASTEXITCODE -ne 0) { Log "fetch failed exit=$LASTEXITCODE"; exit 1 }
$local = (& git rev-parse HEAD).Trim()
$remote = (& git rev-parse wslclone/main).Trim()
if ($local -eq $remote) {
    Log "already up to date ($local)"
} else {
    & git merge wslclone/main -m "pbsd: sync from WSL ($remote)" --no-edit | ForEach-Object { Log $_ }
    if ($LASTEXITCODE -ne 0) { Log "merge failed exit=$LASTEXITCODE"; exit 1 }
}
$ahead = (& git rev-list --count origin/main..HEAD).Trim()
if ([int]$ahead -gt 0) {
    & git push origin main | ForEach-Object { Log $_ }
    if ($LASTEXITCODE -ne 0) { Log "push failed exit=$LASTEXITCODE"; exit 1 }
    Log "pushed $ahead commits to GitHub"
} else {
    Log "nothing to push to GitHub"
}
Log "sync ok"
