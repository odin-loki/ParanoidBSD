# PBSD PROVENANCE LOG

**Author:** Odin Loch  
**Policy:** Every module must have an entry here before implementation begins.  
**Rule:** Spec source must be a public standard, published datasheet, or original design.  
         Never cite an existing implementation's source code.

**Port context:** ParanoidBSD is a full C++23 port of the HardenedBSD kernel tree and KDE Plasma 6 userland. Legacy C components are inventoried in `docs/migration/c_inventory.csv` and retired wave-by-wave; new PBSD code is C++23 modules under `pbsd/`. Index: [docs/README.md](README.md).

## Migration provenance

| Component | Spec source | License | Port note |
|-----------|-------------|---------|-----------|
| HardenedBSD kernel | HardenedBSD Project | BSD-2-Clause | Incremental C→C++23 via waves 4–9; `NORMAL_CXX` in `kern.mk` |
| KDE Plasma 6 | KDE Project | GPL-2.0+ | Wave 3; C++23 already; PBSD theme/compositor integration |
| FreeBSD KPI / kmod ABI | FreeBSD Documentation | BSD-2-Clause | Dual-link via `extern "C"` per `KERNEL_CXX_ABI.md` |

## Entries

| Module | Spec source | Date | Author |
|---|---|---|---|
| pbsd.handles / rights / lineage | Original design — docs/specs/HANDLES.md | 2026-07-19 | Odin Loch |
| pbsd.uda.schema / interp / engine | Original design — docs/specs/UDA_DESCRIPTOR.md | 2026-07-19 | Odin Loch |
| pbsd.uda.virtio_blk | OASIS virtio 1.2 §5.2 (public) | 2026-07-19 | Odin Loch |
| pbsd.analyser | Original design — docs/specs/ANALYSER.md | 2026-07-19 | Odin Loch |
| pbsd.kernel.capsicum_bridge | Original design — docs/specs/CAPSICUM_LINEAGE.md | 2026-07-19 | Odin Loch |
| pbsd_cxx_hello kmod | Original design — docs/specs/KERNEL_CXX_ABI.md | 2026-07-19 | Odin Loch |
| pbsd.bifrost | Original design — SI-8 | 2026-07-19 | Odin Loch |
| pbsd.compositor | Original design — Wayland + Aero | 2026-07-19 | Odin Loch |

## Wave 0 foundation artifacts

| Artifact | Role |
|----------|------|
| `docs/specs/*.md` | Frozen specs for handles, UDA, analyser, ABI, layout, lineage |
| `hbsd/src/sys/conf/kern.mk` | `CXXSTD=c++23`, `NORMAL_CXX` freestanding bundle |
| `pbsd/kernel/pbsd_cxx_hello.*` | Dual-link proof kmod |
| `tools/inventory_c_sources.py` | C legacy inventory → `c_inventory.csv` |
| `tools/ci/run_wave0_analyser.sh` | L1 tidy + SI harness CI driver |
