# C → C++23 migration

Live bookkeeping for the HardenedBSD → PBSD port. Plans and frozen specs live under [`docs/plans/`](../plans/) and [`docs/specs/`](../specs/).

## Inventories

| File | Role |
|------|------|
| [c_inventory.csv](c_inventory.csv) | All C sources, wave tag, LOC estimate |
| [inventory.csv](inventory.csv) | `pbsd.py` driver queue (PENDING / VERIFIED / DEFERRED / …) |
| [batch_progress.json](batch_progress.json) | Stub / converted status per source file |
| [WAVE_STATUS.md](WAVE_STATUS.md) | Burst / wave scoreboard |
| [CONVERTED.md](CONVERTED.md) | Hand-port log for the C++23 nucleus |
| [RESUME_PLAN.md](RESUME_PLAN.md) | WSL driver handoff (queue, auth, what is jammed) |

`clang_port/` is output from `tools/run_todo_passes.py` and `tools/clang_cxx23_port.py` (refusals, staged rewrites, queue, reports). Most of it is gitignored; the reports that are tracked are generated snapshots.

`artifacts/` is per-batch evidence written by `pbsd.py`.

## Subsystem notes

| Document | Topic |
|----------|-------|
| [USERLAND_PORT.md](USERLAND_PORT.md) | Userland / libc porting |
| [DUAL_LINK_KERNEL.md](DUAL_LINK_KERNEL.md) | C and C++23 kmods in one kernel |
| [KDE_CXX23.md](KDE_CXX23.md) | Plasma 6 wave |
| [ZFS_CXX23.md](ZFS_CXX23.md) | ZFS (CDDL module) |
| [LINUXULATOR.md](LINUXULATOR.md) | Linux ABI personality |
| [PROFILES.md](PROFILES.md) | Embedded / server / workstation builds |
| [PURGE_C.md](PURGE_C.md) | When a C TU can be deleted |

## Driver (WSL)

Work from `~/pbsd` on ext4. The Windows tree is the editor checkout.

| Script | Role |
|--------|------|
| `scripts/wsl/pbsd_watchdog.sh` | Keeps the driver alive, deploys `pbsd.py` |
| `scripts/wsl/pbsd_driver.sh` | Mechanical pass, then agent rounds |
| `scripts/wsl/restart_pbsd.sh` | Copy scripts from Windows, restart watchdog |
| `Watch-PBSD.bat` | Progress console (use `attach` if the driver is already up) |
| `Stop-PBSD.bat` | Kill watchdog / driver / agents |

```bash
python3 pbsd.py --status
python3 pbsd.py --mechanical-only --mech-jobs 8
python3 pbsd.py --jobs 18 --gate-jobs 4 --no-mechanical
```

Mechanical resume state (local, gitignored): `.mech_pass_done`, `.mech_checkpoint.jsonl`.
