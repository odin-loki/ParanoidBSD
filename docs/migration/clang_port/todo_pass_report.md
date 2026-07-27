# PBSD todo.md Pass Report

Generated: `2026-07-27T11:40:16+00:00`

- Files processed: **30**
- Edits applied: **533**
- Refusals (model queue only): **0**
- compile_commands coverage: **100.0%**
- Corpus OK: **True**
- IR equal: **0** / ran **2**
- Diff equal: **1**

## Reason histogram (true refusals — not successful edits)

| Reason | Count |
|---|---:|

## Top edited files

| Edits | Refusals | File |
|---:|---:|---|
| 66 | 0 | `hbsd/src/bin/ed/main.c` |
| 55 | 0 | `hbsd/src/bin/df/df.c` |
| 50 | 0 | `hbsd/src/bin/chio/chio.c` |
| 47 | 0 | `hbsd/src/bin/dd/args.c` |
| 33 | 0 | `hbsd/src/bin/cat/cat.c` |
| 30 | 0 | `hbsd/src/bin/cp/cp.c` |
| 27 | 0 | `hbsd/src/bin/date/date.c` |
| 22 | 0 | `hbsd/src/bin/ed/glbl.c` |
| 22 | 0 | `hbsd/src/bin/ed/io.c` |
| 19 | 0 | `hbsd/src/bin/ed/buf.c` |
| 17 | 0 | `hbsd/src/bin/date/vary.c` |
| 17 | 0 | `hbsd/src/bin/ed/re.c` |
| 17 | 0 | `hbsd/src/bin/ed/sub.c` |
| 16 | 0 | `hbsd/src/bin/kenv/kenv.c` |
| 15 | 0 | `hbsd/src/bin/getfacl/getfacl.c` |
| 14 | 0 | `hbsd/src/bin/cp/utils.c` |
| 14 | 0 | `hbsd/src/bin/dd/dd.c` |
| 10 | 0 | `hbsd/src/bin/csh/iconv_stub.c` |
| 8 | 0 | `hbsd/src/bin/ed/undo.c` |
| 6 | 0 | `hbsd/src/bin/chmod/chmod.c` |
| 5 | 0 | `hbsd/src/bin/cpuset/cpuset.c` |
| 4 | 0 | `hbsd/src/bin/chflags/chflags.c` |
| 3 | 0 | `hbsd/src/bin/dd/conv.c` |
| 3 | 0 | `hbsd/src/bin/dd/misc.c` |
| 3 | 0 | `hbsd/src/bin/dd/position.c` |
| 3 | 0 | `hbsd/src/bin/echo/echo.c` |
| 3 | 0 | `hbsd/src/bin/hostname/hostname.c` |
| 2 | 0 | `hbsd/src/bin/domainname/domainname.c` |
| 1 | 0 | `hbsd/src/bin/dd/conv_tab.c` |
| 1 | 0 | `hbsd/src/bin/dd/gen.c` |

## Outputs

- Staged C++: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/staged`
- Refusals: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 37597)

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 10030 |
| `STR_FORMAT_CANDIDATE` | 3798 |
| `MALLOC_ESCAPE` | 3058 |
| `SPAN_CANDIDATE` | 2840 |
| `NULLABILITY` | 2601 |
| `LIFETIME_OWN` | 1732 |
| `MACRO_CONSTEXPR` | 1730 |
| `LIFETIME_FAIL` | 1727 |
| `ANDERSEN_OWNED` | 1695 |
| `GLOBAL_CLUSTER` | 1444 |
| `POINTER_KIND` | 1401 |
| `ANDERSEN_ESCAPE` | 1055 |
| `MACRO_DIVERGENT` | 977 |
| `DEAD_STORE` | 711 |
| `SYSCALL_TRACE` | 616 |
| `PURITY` | 537 |
| `GOTO_CLEANUP_CANDIDATE` | 266 |
| `KR_DEFINITION` | 250 |
| `VLA` | 246 |
| `LOCK_DISCIPLINE` | 186 |
| `QUEUE_H_SITE` | 178 |
| `TYPEDEF_COMPLEX` | 176 |
| `CALL_SITE_SPAN` | 103 |
| `RANGE_FOR_CANDIDATE` | 90 |
| `CALLBACK_CTX` | 57 |
| `GOTO_CROSS_INIT` | 29 |
| `FLEXIBLE_ARRAY` | 24 |
| `BIT_CAST_CANDIDATE` | 16 |
| `FN_PTR_STRUCT` | 11 |
| `ENUM_ARITHMETIC` | 7 |
| `NESTED_STRUCT_TAG` | 5 |
| `GENERIC` | 1 |

