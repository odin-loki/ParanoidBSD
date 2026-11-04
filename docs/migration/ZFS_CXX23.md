# OpenZFS C++23 Port Track

**Status:** Wave 6 scaffold  
**Upstream:** `hbsd/src/sys/contrib/openzfs` (C, stays dual-linked during migration)

## Goal

Port the ZFS user/kernel boundary to typed C++23 modules under `pbsd/zfs/` while
keeping the existing OpenZFS C core callable via `extern "C"` shims until each
subsystem is converted file-by-file per `docs/migration/c_inventory.csv`.

## Module map

| Module | Role | hbsd anchor |
|--------|------|-------------|
| `pbsd.zfs` | Pool import, dataset handles, capability-checked mount | `sys/cddl/contrib/opensolaris/uts/common/fs/zfs/` |
| `pbsd.fs` | VFS vnode façade shared with non-ZFS filesystems | `sys/kern/vfs_*.c` |
| `pbsd.geom` | GEOM provider glue for ZVOL | `sys/geom/` |

## Migration phases

1. **Scaffold (Wave 6)** — `pbsd.zfs` stub with `DatasetHandle` + rights checks; no OpenZFS calls yet.
2. **Shim layer** — `extern "C"` wrappers around `zfs_ioctl`, `dmu_*` entry points (dual-link like `pbsd/kernel/shims/`).
3. **Incremental conversion** — move leaf utilities (checksum, nvlist parsing helpers) to freestanding C++23 TUs.
4. **Integration** — wire `pbsd.zfs` into kernel module build via `NORMAL_CXX` in `kmod.mk`.

## Build flags

All modules importing `pbsd.handles` compile with:

```
-fno-exceptions -fno-rtti -ffreestanding
```

## Invariants

- **SI-1:** dataset rights only narrow on grant/delegation.
- **SI-4:** kmem backing ZFS arc buffers uses zero-initialised allocations (`pbsd.kernel.alloc`).
- **SI-6:** revoking a `DatasetHandle` invalidates derived snapshots/clones in the lineage tree.

## References

- `docs/specs/HANDLES.md`
- `docs/specs/KERNEL_CXX_ABI.md`
- `docs/security/INVARIANTS.md` (SI-1, SI-4, SI-6)
