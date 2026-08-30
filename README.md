# ParanoidBSD (PBSD)

A C++23 operating system ported from [HardenedBSD](https://hardenedbsd.org/) 15-STABLE, with KDE Plasma 6 as the desktop and a capability / handle security nucleus on top of what HBSD already ships.

New and rewritten code lives under `pbsd/` as C++23 modules. The HardenedBSD tree in `hbsd/` is the behavioural specification: ports must match it, including bugs, until a change is logged separately.

Documentation index: [`docs/README.md`](docs/README.md).

## What this repo is

| Tree | Role |
|------|------|
| `pbsd/` | PBSD C++23 modules — kernel nucleus, userland ports, UDA, BIFROST, compositor, theme |
| `hbsd/` | HardenedBSD 15-STABLE source (the original) |
| `kde/` | Plasma 6 / KWin / frameworks used by the desktop wave |
| `tools/` | Inventory, deterministic rewrite passes, DeepSeek agent port, Clang helpers |
| `docs/` | Specs, security model, migration status, plans |
| `scripts/` | WSL driver, watchdog, and the Windows progress console |

License stays BSD for HBSD-derived code. Every module needs an entry in [`docs/PROVENANCE.md`](docs/PROVENANCE.md) before it is treated as done.

## How the port works

1. **Inventory** — `tools/inventory_c_sources.py` / `tools/clang_cxx23_port.py` score files and write `docs/migration/c_inventory.csv`.
2. **Deterministic passes** — `tools/run_todo_passes.py` applies safe rewrites (tiers 0–4). Failures go to `docs/migration/clang_port/refusals.jsonl`.
3. **Agent loop** — `pbsd.py` (DeepSeek Flash → Pro, max effort) fills stubbed / refused files. Compile, ASan/UBSan, differential, and IR checks still run; the model does not self-certify.
4. **Bookkeeping** — updates `docs/migration/batch_progress.json`; failures land in `agent_port_failures.jsonl`.

Plans: [C++23 port](docs/plans/cxx23-port-master-plan.md) · [agent port](docs/plans/agent-port-master-plan.md) · [pass checklist](docs/plans/todo-passes.md).

## Run the migration driver

Preferred on Windows (uses `secrets/api-keys`):

```powershell
python pbsd.py --status
python pbsd.py --dry-run --scope bin --limit 20
python pbsd.py --scope bin,usr.bin
.\scripts\run_agent_port_deepseek.ps1 -Scope "bin"
```

WSL live driver (ext4 checkout `~/pbsd`):

```bat
Watch-PBSD.bat attach
```

Or: `bash scripts/wsl/restart_pbsd.sh`. Stop: `Stop-PBSD.bat`.

Defaults: 48 Flash + 24 Pro workers, `reasoning_effort=max`.

## Run the tools locally

```bash
# Deterministic passes (no API key)
python3 tools/run_todo_passes.py --corpus-only
python3 tools/run_todo_passes.py --scope bin --limit 80

# Agent port (same as pbsd.py) — key from secrets/api-keys
copy secrets\api-keys.example secrets\api-keys
python pbsd.py --self-test
python pbsd.py --dry-run --scope bin/echo --limit 5
```

Hosted C++23 nucleus (Clang 18+, CMake 3.28+):

```bash
cmake -S pbsd -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Non-negotiables

- No file is done until differential or IR verification passes. Compile-only is unverified.
- Port faithfully. Do not silently “fix” bugs in the original.
- Deterministic tooling first; models fill what remains (stubbed inventory / refusals).
- Kernel C++ is freestanding: `-fno-exceptions -fno-rtti` ([`docs/specs/KERNEL_CXX_ABI.md`](docs/specs/KERNEL_CXX_ABI.md)).

## Status

Wave and burst history: [`docs/migration/WAVE_STATUS.md`](docs/migration/WAVE_STATUS.md).  
Converted nucleus log: [`docs/migration/CONVERTED.md`](docs/migration/CONVERTED.md).  
Driver resume notes: [`docs/migration/RESUME_PLAN.md`](docs/migration/RESUME_PLAN.md).
