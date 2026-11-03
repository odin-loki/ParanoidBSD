# llama-server for GLM-5.3-Flash on the P920 (Windows, native CUDA build of llama.cpp from PR #27752).
# Keeps the server up: if llama-server exits or stops answering /health, it is restarted.
#   powershell -ExecutionPolicy Bypass -File glm-server.ps1 -Quant IQ2_XXS     # 64 GB stage
#   powershell -ExecutionPolicy Bypass -File glm-server.ps1 -Quant Q4_K_M      # after the RAM upgrade
param(
  [ValidateSet("IQ2_XXS", "Q4_K_M")] [string]$Quant = "Q4_K_M",
  [string]$Models = "D:\models\GLM-5.3-Flash",
  [string]$Server = "D:\llama.cpp\build\bin\Release\llama-server.exe",
  [int]$Port = 8080,
  [int]$Slots = 0,          # 0 = default for the quant
  [int]$CtxPerSlot = 32768,
  [switch]$Once             # start once without the restart loop (pbsd_run then does the restarts)
)
$ErrorActionPreference = "Stop"
# First shard of a split GGUF, or the single file.
$model = Get-ChildItem $Models -Recurse -Filter "*$Quant*.gguf" | Sort-Object Name | Select-Object -First 1
if (-not $model) { throw "no $Quant GGUF under $Models" }

if ($Quant -eq "IQ2_XXS") {
  # 64 GB RAM + 24 GB VRAM. The file is about 85 GB (estimate: 320B x 2.06 bits): it does not fit in
  # RAM, so it is memory-mapped and part of it is read from D: on every token. Use this stage to
  # calibrate the harness on about 50 tasks, not for the run.
  if ($Slots -eq 0) { $Slots = 2 }
  $gpu = @("-ngl", "999", "--n-cpu-moe", "999")   # attention, shared experts and KV on the GPU; routed experts in RAM
  $threads = 30; $mem = @()                        # no --mlock: it cannot all be locked
  $kv = @("--cache-type-k", "q8_0", "--cache-type-v", "q8_0")
} else {
  # 512 GB-2 TB RAM. The file is about 195 GB (estimate: 320B x 4.85 bits), held in RAM with --mlock.
  if ($Slots -eq 0) { $Slots = 4 }
  $gpu = @("-ngl", "999", "--n-cpu-moe", "999")
  $threads = 24; $mem = @("--mlock")               # 8 cores left for builds, proofs and QEMU
  $kv = @("--cache-type-k", "q8_0", "--cache-type-v", "q8_0")
}
$largs = @(
  "-m", $model.FullName, "--host", "127.0.0.1", "--port", "$Port",
  "--parallel", "$Slots", "--ctx-size", "$($Slots * $CtxPerSlot)",
  "--threads", "$threads", "--threads-batch", "32", "--batch-size", "2048", "--ubatch-size", "512",
  "-fa", "on", "--jinja", "--cache-reuse", "256", "--metrics", "--slot-save-path", "$Models\slots"
) + $gpu + $mem + $kv
# NUMA: llama.cpp's --numa is effective on Linux only. On Windows, set "NUMA nodes per socket" and
# memory interleaving in the P920 BIOS; on Linux use glm-server.sh, which adds --numa distribute.
New-Item -ItemType Directory -Force "$Models\slots", "$Models\logs" | Out-Null

# A hung server from an earlier run is killed first, so a restart always starts clean.
Get-Process llama-server -ErrorAction SilentlyContinue | Stop-Process -Force
do {
  $log = "$Models\logs\llama-server-$(Get-Date -Format yyyyMMdd-HHmmss).log"
  Write-Host "starting llama-server ($Quant, $Slots slots, $CtxPerSlot ctx each) -> $log"
  $p = Start-Process -FilePath $Server -ArgumentList $largs -NoNewWindow -PassThru -RedirectStandardError $log
  if ($Once) { $p.WaitForExit(); exit $p.ExitCode }   # pbsd_run watches this process
  $bad = 0
  while (-not $p.HasExited) {
    Start-Sleep -Seconds 30
    try { $r = Invoke-WebRequest -UseBasicParsing -TimeoutSec 20 "http://127.0.0.1:$Port/health"; $bad = 0 }
    catch { if ($_.Exception.Response.StatusCode.value__ -ne 503) { $bad++ } }   # 503 = still loading
    if ($bad -ge 6) { Write-Host "no /health answer for 3 minutes: killing"; Stop-Process -Id $p.Id -Force }
  }
  Write-Host "llama-server exited ($($p.ExitCode)); restarting in 10 s"
  Start-Sleep -Seconds 10
} while ($true)
