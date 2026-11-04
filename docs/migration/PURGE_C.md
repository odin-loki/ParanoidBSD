# C Purge — PBSD 0.1 Exit Criteria (Wave 9)

**Status:** Scaffold — gate enforced by `tools/wave_purge_c_check.py --gate`.

## Definition

**Owned C** means `.c` translation units that PBSD builds or ships as part of the native tree (`hbsd/src`, `kde/`, and future `pbsd/` bridges). Third-party firmware blobs and hand-written assembly stubs are excluded via `c_purge_exceptions.txt`.

## Exit criteria for PBSD 0.1

1. **Inventory zero:** `docs/migration/c_inventory.csv` contains no rows assigned to active conversion waves (`wave2`–`wave8`) — all converted or reclassified.
2. **Owned `.c` count zero:** After applying firmware/asm exceptions, no owned `.c` files remain on disk under inventoried roots.
3. **Gate green:** `python3 tools/wave_purge_c_check.py --gate` exits 0 in CI.
4. **Build graphs:** CMake / kernel build graphs link no PBSD-owned `.c` except entries on the exception list.
5. **Analyser:** All replacement C++23 modules pass L1–L4 (see `docs/specs/ANALYSER.md`).

## Exception policy

Exceptions are explicit path globs in `docs/migration/c_purge_exceptions.txt`:

- Boot firmware (UEFI, ACPI tables)
- `.S` / `.asm` companions referenced from exception entries
- Vendor blobs with no maintainable C++23 path (documented in PROVENANCE)

Adding an exception requires a one-line rationale comment in that file.

## Workflow

```bash
python3 tools/inventory_c_sources.py
python3 tools/wave_purge_c_check.py          # report
python3 tools/wave_purge_c_check.py --gate   # CI fail if criteria unmet
```

## Full C purge gate

PBSD 0.1 release is blocked until the inventory reports **zero owned `.c`** (exceptions subtracted). Wave 9 scaffold is complete when tooling and docs exist; the release gate fires only when the count hits zero.
