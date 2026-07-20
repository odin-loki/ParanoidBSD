# SPEC: C++23 Module Layout

**Status:** Frozen for Wave 0–8  
**Port context:** ParanoidBSD full C++23 port — BMIs partition the tree; dependency law prevents cycles and hosted code in kernel runtime.

## Module map

| Module | Path | Freestanding | Wave |
|--------|------|--------------|------|
| `pbsd.core` | `pbsd/core/` | Yes | 0–1 |
| `pbsd.rights` | `pbsd/rights/` | Yes | 1 |
| `pbsd.handles` | `pbsd/handles/` | Yes | 1 |
| `pbsd.cap.lineage` | `pbsd/lineage/` | Yes | 1 |
| `pbsd.uda.schema` | `pbsd/uda/` | Yes | 1 |
| `pbsd.uda.interp` | `pbsd/uda/` | Yes | 1 |
| `pbsd.uda.engine` | `pbsd/uda/` | Yes | 1 |
| `pbsd.uda.virtio_blk` | `pbsd/uda/descriptors/` | Yes | 5 |
| `pbsd.kernel.capsicum` | `pbsd/kernel/` | Yes | 4 |
| `pbsd.userland.capsicum` | `pbsd/userland/` | No | 2 |
| `pbsd.net` | `pbsd/net/` | Yes | 6 |
| `pbsd.fs` | `pbsd/fs/` | Yes | 6 |
| `pbsd.stand` | `pbsd/stand/` | Yes | 7 |
| `pbsd.bifrost` | `pbsd/bifrost/` | Mixed | 8 |
| `pbsd.compositor` | `pbsd/compositor/` | No | 8 |
| `pbsd.analyser` | `pbsd/analyser/` | No (hosted Clang) | 1+ |

## Dependency law

```
pbsd.core
  └── pbsd.rights ── pbsd.handles ── pbsd.cap.lineage
  └── pbsd.uda.schema ── pbsd.uda.interp ── pbsd.uda.engine
```

Hard rules:

1. `uda.schema` must not import engines or handles (descriptors are data-only).
2. Analyser must not link into kernel runtime.
3. Compositor / theme may import handles; kernel may not import compositor.
4. Freestanding modules: `-fno-exceptions -fno-rtti` on all BMI consumers (see root `CMakeLists.txt`).

## Build surfaces

| Surface | Build system | C++ std |
|---------|--------------|---------|
| PBSD nucleus (dev/CI) | CMake 3.28+ | C++23 modules |
| HardenedBSD kmods | `bsd.kmod.mk` + `NORMAL_CXX` | C++23 freestanding |
| KDE Plasma 6 | upstream CMake | C++23 (wave 3) |

## File naming

- Interface units: `pbsd.<area>.cppm`
- Kernel dual-link glue: `pbsd_<area>_<name>.{h,c,cpp}` in `pbsd/kernel/`
- Descriptors: `pbsd/uda/descriptors/<device>.cppm`

## Inventory coupling

`tools/inventory_c_sources.py` assigns legacy `.c` files to conversion waves. New code lands as `.cppm` / `.cpp` under `pbsd/` first, then replaces inventory rows wave-by-wave.
