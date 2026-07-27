# PBSD Clang C→C++23 Port Report

Generated: `2026-07-27T11:08:22+00:00`

## Tool availability

| Tool | Found |
|---|---|
| `clang` | `/usr/bin/clang-18` |
| `clangxx` | `/usr/bin/clang++-18` |
| `clang_tidy` | `/usr/bin/clang-tidy-18` |
| `clang_query` | `/usr/bin/clang-query-18` |
| `clang_format` | `MISSING` |
| `clang_scan_deps` | `/usr/bin/clang-scan-deps-18` |
| `clang_apply` | `/usr/bin/clang-apply-replacements-18` |

## Inventory

- Rows in `c_inventory.csv`: **28376**
- Files scored in AST census: **200**

### Risk tier distribution

- Tier 1: **34**
- Tier 2: **52**
- Tier 3: **114**

### Envelope hints

- Envelope A: **1**
- Envelope B: **5**
- Envelope C: **194**

### Hottest files (port with care)

| Score | Tier | Env | Path |
|---:|---|---|---|
| 196 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_aggregate.c` |
| 187 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_consume.c` |
| 152 | 1 | C | `hbsd/src/bin/sh/parser.c` |
| 145 | 1 | B | `hbsd/src/bin/ps/ps.c` |
| 142 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/cmd/lockstat/lockstat.c` |
| 131 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_parser.c` |
| 114 | 1 | B | `hbsd/src/bin/pkill/pkill.c` |
| 110 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_link.c` |
| 109 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_open.c` |
| 103 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/cmd/dtrace/dtrace.c` |
| 100 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_module.c` |
| 93 | 1 | C | `hbsd/src/bin/chio/chio.c` |
| 89 | 1 | C | `hbsd/src/bin/sh/eval.c` |
| 89 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_cc.c` |
| 87 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_map.c` |
| 79 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_ident.c` |
| 75 | 1 | C | `hbsd/src/bin/pax/tables.c` |
| 69 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_options.c` |
| 67 | 1 | C | `hbsd/src/bin/df/df.c` |
| 65 | 1 | C | `hbsd/src/bin/dd/args.c` |
| 64 | 1 | C | `hbsd/src/bin/ls/ls.c` |
| 64 | 1 | C | `hbsd/src/bin/sh/jobs.c` |
| 59 | 1 | C | `hbsd/src/bin/ls/print.c` |
| 59 | 1 | C | `hbsd/src/bin/sh/expand.c` |
| 57 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/cmd/dtrace/test/tst/common/nfs/tst.call3.c` |

## clang-tidy

- Targets: **0**
- Warning sum: **0**
- compile_commands: `None`
- fix mode: **None**

## Queue

- Items: **200** → `docs/migration/clang_port/queue.json`

## Deps sample

- Mode: `n/a`

## Next deterministic steps

1. Drain `queue.json` leaves (tier 3) through Stages A–H in the master plan.
2. Install/fix `compile_commands.json` (`cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) then re-run `--phase tidy --fix`.
3. Wire LibTooling `pbsd-ownership` plugin for L2 as handle usage grows.
4. Keep `python3 tools/wave_purge_c_check.py --gate` in CI until owned C hits zero.
5. Use clang-query matchers in `tools/clang_port/` to expand ownership-candidate hunts.

## Other deterministic ideas (not yet automated here)

- `clang-include-cleaner` / IWYU on each DONE file
- `clang-diff` structural compare original vs port
- Sanitizer+replay differential runner as Stage G oracle
- TLA+/SPIN for epoch substrate only
- Reproducible-build hash gate + SBOM per release

