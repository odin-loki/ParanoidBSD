# PBSD Clang C→C++23 Port Report

Generated: `2026-07-27T12:01:44+00:00`

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
- Files scored in AST census: **300**

### Risk tier distribution

- Tier 1: **52**
- Tier 2: **80**
- Tier 3: **168**

### Envelope hints

- Envelope A: **1**
- Envelope B: **8**
- Envelope C: **291**

### Hottest files (port with care)

| Score | Tier | Env | Path |
|---:|---|---|---|
| 8902 | 1 | C | `hbsd/src/lib/clang/liblldb/LLDBWrapLua.cpp` |
| 196 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_aggregate.c` |
| 187 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_consume.c` |
| 167 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_printf.c` |
| 152 | 1 | C | `hbsd/src/bin/sh/parser.c` |
| 145 | 1 | B | `hbsd/src/bin/ps/ps.c` |
| 144 | 1 | B | `hbsd/src/lib/lib80211/lib80211_regdomain.c` |
| 142 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/cmd/lockstat/lockstat.c` |
| 131 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_parser.c` |
| 116 | 1 | C | `hbsd/src/lib/geom/part/geom_part.c` |
| 114 | 1 | B | `hbsd/src/bin/pkill/pkill.c` |
| 110 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_link.c` |
| 109 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_open.c` |
| 105 | 1 | B | `hbsd/src/lib/libbe/be.c` |
| 103 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/cmd/dtrace/dtrace.c` |
| 100 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_module.c` |
| 98 | 1 | C | `hbsd/src/lib/geom/eli/geom_eli.c` |
| 97 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/tools/ctf/cvt/dwarf.c` |
| 95 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/tools/ctf/cvt/ctf.c` |
| 93 | 1 | C | `hbsd/src/bin/chio/chio.c` |
| 90 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/tools/ctf/cvt/merge.c` |
| 89 | 1 | C | `hbsd/src/bin/sh/eval.c` |
| 89 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_cc.c` |
| 87 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_map.c` |
| 79 | 1 | C | `hbsd/src/cddl/contrib/opensolaris/lib/libdtrace/common/dt_ident.c` |

## clang-tidy

- Targets: **0**
- Warning sum: **0**
- compile_commands: `None`
- fix mode: **None**

## Queue

- Items: **300** → `docs/migration/clang_port/queue.json`

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

