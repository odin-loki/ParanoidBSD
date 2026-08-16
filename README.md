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
| `tools/` | Inventory, deterministic rewrite passes, agent port loop, Clang helpers |
| `docs/` | Specs, security model, migration status, plans |
| `scripts/` | WSL driver, watchdog, and the Windows progress console |

License stays BSD for HBSD-derived code. Every module needs an entry in [`docs/PROVENANCE.md`](docs/PROVENANCE.md) before it is treated as done.

## How the port works

1. **Inventory** — `tools/inventory_c_sources.py` / `tools/clang_cxx23_port.py` score files and write `docs/migration/c_inventory.csv`.
2. **Deterministic passes** — `tools/run_todo_passes.py` applies safe rewrites (tiers 0–4). Failures go to `docs/migration/clang_port/refusals.jsonl`.
3. **Mechanical IR sweep** — `pbsd.py --mechanical-only` tries a no-agent C→C++ port and accepts it only if LLVM IR matches.
4. **Agent loop** — `pbsd.py` (Composer / escalate) or `tools/pbsd_agent_port.py` (DeepSeek / Kimi) fill what the passes refused. Compile, ASan/UBSan, differential, and IR checks still run; the model does not self-certify.
5. **Bookkeeping** — verified batches update `docs/migration/inventory.csv` and `docs/migration/batch_progress.json`.

Plans: [C++23 port](docs/plans/cxx23-port-master-plan.md) · [agent port](docs/plans/agent-port-master-plan.md) · [pass checklist](docs/plans/todo-passes.md).

## Run the migration driver (WSL)

The live driver runs in WSL on ext4 (`~/pbsd`), not on the Windows/OneDrive checkout.

```bat
Watch-PBSD.bat attach
```

That opens the progress console only. To deploy scripts and start the watchdog from WSL:

```bash
bash scripts/wsl/restart_pbsd.sh
```

Stop everything: `Stop-PBSD.bat`.

`pbsd.py` defaults: 18 agent jobs, 4 harness builds, 8 mechanical IR workers (capped — 64 parallel clang jobs OOM the WSL VM).

## Run the tools locally

```bash
# Deterministic passes (no API key)
python3 tools/run_todo_passes.py --corpus-only
python3 tools/run_todo_passes.py --scope bin --limit 80

# Agent port loop (Stage F/G) — needs DEEPSEEK_API_KEY / MOONSHOT_API_KEY
cp .env.example .env   # then fill keys; never commit .env
python3 tools/pbsd_agent_port.py --self-test
python3 tools/pbsd_agent_port.py --dry-run --scope bin/echo
```

Hosted C++23 nucleus (Clang 18+, CMake 3.28+):

```bash
cmake -S pbsd -B build -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Non-negotiables

- No file is done until differential or IR verification passes. Compile-only is unverified.
- Port faithfully. Do not silently “fix” bugs in the original.
- Deterministic tooling first; models only consume the refusal queue.
- Kernel C++ is freestanding: `-fno-exceptions -fno-rtti` ([`docs/specs/KERNEL_CXX_ABI.md`](docs/specs/KERNEL_CXX_ABI.md)).

## Status

Wave and burst history: [`docs/migration/WAVE_STATUS.md`](docs/migration/WAVE_STATUS.md).  
Converted nucleus log: [`docs/migration/CONVERTED.md`](docs/migration/CONVERTED.md).  
Driver resume notes: [`docs/migration/RESUME_PLAN.md`](docs/migration/RESUME_PLAN.md).
