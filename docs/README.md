# PBSD documentation

Start at the [repository README](../README.md). This tree is the rest of the written design.

| Path | What it is |
|------|------------|
| [GLOSSARY.md](GLOSSARY.md) | Project terms |
| [PROVENANCE.md](PROVENANCE.md) | Spec source / license trail for every module |
| [plans/](plans/) | How the port is supposed to run |
| [specs/](specs/) | Frozen module contracts |
| [security/](security/) | Threat model and invariants |
| [migration/](migration/) | Wave status, subsystem notes, live driver state |

Vendor docs under `hbsd/` and `kde/` stay with those trees. Do not move them here.

## Plans

| Document | Role |
|----------|------|
| [cxx23-port-master-plan.md](plans/cxx23-port-master-plan.md) | Architecture, envelopes, stages A–H, Clang-first doctrine |
| [agent-port-master-plan.md](plans/agent-port-master-plan.md) | Stage F/G DeepSeek session (`pbsd.py`) |
| [todo-passes.md](plans/todo-passes.md) | Deterministic pass checklist (tiers 0–4) |

## Specs (frozen)

| Document | Module |
|----------|--------|
| [MODULE_LAYOUT.md](specs/MODULE_LAYOUT.md) | C++23 module map and dependency law |
| [KERNEL_CXX_ABI.md](specs/KERNEL_CXX_ABI.md) | Freestanding kernel ABI, dual-link C |
| [HANDLES.md](specs/HANDLES.md) | Unique / borrowed / shared handles |
| [CAPSICUM_LINEAGE.md](specs/CAPSICUM_LINEAGE.md) | Capability revocation tree |
| [UDA_DESCRIPTOR.md](specs/UDA_DESCRIPTOR.md) | Universal Driver Architecture |
| [ANALYSER.md](specs/ANALYSER.md) | Ownership analyser (L1–L4) |

## Security

| Document | Role |
|----------|------|
| [THREAT_MODEL.md](security/THREAT_MODEL.md) | STRIDE surfaces (SI-1 … SI-8) |
| [INVARIANTS.md](security/INVARIANTS.md) | Runtime invariants the port must keep |

## Migration

See [migration/README.md](migration/README.md) for the driver, inventories, and subsystem notes.

Component READMEs that stay next to code:

- [`pbsd/theme/README.md`](../pbsd/theme/README.md) — Aero / Plasma theme
- [`pbsd/compositor/README.md`](../pbsd/compositor/README.md) — Wayland compositor
- [`tools/pbsd_passes/README.md`](../tools/pbsd_passes/README.md) — rewrite passes
- [`tools/clang_port/README.md`](../tools/clang_port/README.md) — clang-tidy / query configs
