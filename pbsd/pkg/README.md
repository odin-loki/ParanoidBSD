# PBSD Atomic Packaging (Wave 9)

Content-addressed package transactions for rollback-safe system updates.

## Layout

| Path | Role |
|------|------|
| `pbsd/pkg/pbsd.pkg.cppm` | C++23 module — `AtomicUpdater` transaction API |
| `tools/pbsd_pkg/` | CLI wrapper (future) calling the module via thin host binary |

## Transaction model

1. `begin()` — open staging transaction
2. `stage()` — append content-addressed package refs (name + digest)
3. `commit()` — apply atomically (ZFS boot environment swap in full implementation)
4. `rollback()` — revert to prior boot env on failure

## PBSD 0.1 target

`pbsd-pkg apply manifest.pbsp` applies a signed manifest; power-loss mid-commit rolls back to last good boot env.
