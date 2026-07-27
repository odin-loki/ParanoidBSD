# PBSD todo.md Pass Report

Generated: `2026-07-27T10:12:01+00:00`

- Files processed: **4**
- Edits applied: **217**
- Refusals (model queue only): **0**
- compile_commands coverage: **100.0%**
- Corpus OK: **True**
- IR equal: **0** / ran **0**
- Diff equal: **0**

## Reason histogram (true refusals — not successful edits)

| Reason | Count |
|---|---:|

## Top edited files

| Edits | Refusals | File |
|---:|---:|---|
| 95 | 0 | `hbsd/src/lib/libc/resolv/res_findzonecut.c` |
| 54 | 0 | `hbsd/src/lib/libc/db/btree/bt_split.c` |
| 40 | 0 | `hbsd/src/usr.bin/sort/radixsort.c` |
| 28 | 0 | `hbsd/src/usr.sbin/ppp/main.c` |

## Outputs

- Staged C++: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/staged`
- Refusals: `/mnt/c/Users/odinl/OneDrive/Desktop/Operating System/docs/migration/clang_port/refusals.jsonl`
- Full JSON: `docs/migration/clang_port/pass_report.json`

## Proposal histogram (`proposals.jsonl`, 31007)

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 7675 |
| `STR_FORMAT_CANDIDATE` | 3574 |
| `MALLOC_ESCAPE` | 2580 |
| `SPAN_CANDIDATE` | 2335 |
| `NULLABILITY` | 2129 |
| `LIFETIME_OWN` | 1610 |
| `ANDERSEN_OWNED` | 1601 |
| `LIFETIME_FAIL` | 1435 |
| `GLOBAL_CLUSTER` | 1228 |
| `MACRO_CONSTEXPR` | 1192 |
| `POINTER_KIND` | 1164 |
| `ANDERSEN_ESCAPE` | 966 |
| `MACRO_DIVERGENT` | 776 |
| `DEAD_STORE` | 633 |
| `SYSCALL_TRACE` | 610 |
| `PURITY` | 422 |
| `VLA` | 234 |
| `GOTO_CLEANUP_CANDIDATE` | 233 |
| `QUEUE_H_SITE` | 163 |
| `TYPEDEF_COMPLEX` | 128 |
| `RANGE_FOR_CANDIDATE` | 84 |
| `CALL_SITE_SPAN` | 83 |
| `CALLBACK_CTX` | 48 |
| `GOTO_CROSS_INIT` | 29 |
| `LOCK_DISCIPLINE` | 18 |
| `BIT_CAST_CANDIDATE` | 16 |
| `KR_DEFINITION` | 11 |
| `FN_PTR_STRUCT` | 9 |
| `FLEXIBLE_ARRAY` | 8 |
| `ENUM_ARITHMETIC` | 7 |
| `NESTED_STRUCT_TAG` | 5 |
| `GENERIC` | 1 |

