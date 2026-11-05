# llama-server for the PBSD run

GLM-5.3-Flash (320B total, 18B active, MoE) runs on the P920 under llama.cpp built from PR #27752, through llama-server's OpenAI-compatible API at `http://127.0.0.1:8080/v1`. `pbsd_run` talks to it, health-checks it every 30 s and restarts it when it crashes or hangs.

| File | Use |
|---|---|
| `glm-server.ps1` | Windows (native CUDA build). Restart loop by default; `-Once` when `pbsd_run` supervises it. |
| `glm-server.sh` | Linux boot or WSL2 with the model on ext4. Adds `--numa distribute` and `numactl --interleave=all`. |
| `config.iq2xxs.json` | `$PBSD_STATE/config.json` for 64 GB with IQ2_XXS: 2 slots, calibration only. |
| `config.q4km.json` | `$PBSD_STATE/config.json` for 512 GB–2 TB with Q4_K_M: 4 slots, the run. |

## Layout on Windows

```
D:\models\GLM-5.3-Flash\          GGUF shards (IQ2_XXS and Q4_K_M), slots\, logs\
D:\llama.cpp\build\bin\Release\   llama-server.exe (CUDA, from PR #27752)
D:\llama.cpp\pbsd\glm-server.ps1  copy of this script
```

`pbsd_run` and the gates need Linux (fork, tmpfs, numactl), so they run in WSL2. Set `networkingMode=mirrored` in `%UserProfile%\.wslconfig` so `127.0.0.1:8080` in WSL2 reaches the Windows server, and `memory=` high enough for the tmpfs and gates. `server_cmd` starts the Windows script through WSL interop (`powershell.exe`); the script kills any old `llama-server.exe` first, so a restart after a hang is clean.

## Flags

| Flag | Why |
|---|---|
| `-ngl 999 --n-cpu-moe 999` | Attention, shared weights and KV cache on the RTX 3090; routed experts in system RAM. If VRAM is left over, lower `--n-cpu-moe` to move some expert layers to the GPU. |
| `--parallel N --ctx-size N×32768` | One slot per agent; each slot gets 32k tokens. `local_slots` in config.json must equal N. |
| `--cache-reuse 256`, `cache_prompt`, `id_slot` | Each agent stays on one slot, so its prompt prefix is not recomputed every turn. |
| `--cache-type-k/v q8_0`, `-fa on` | Halves KV memory on the GPU. |
| `--threads 24` (Q4_K_M) | Leaves 8 cores for builds, proofs and QEMU. |
| `--mlock` (Q4_K_M only) | Weights stay in RAM. Not for IQ2_XXS on 64 GB: the file is bigger than RAM. |
| `--jinja` | Uses the model's chat template for tool calls. |
| `--numa distribute` | Linux only. On Windows llama.cpp has no NUMA placement; set memory interleaving in the BIOS or boot Linux for the run. |

All sizes and speeds here are estimates until measured on the P920: run `llama-bench` and a 50-task calibration (AGENTS.md section 8.4) first.
