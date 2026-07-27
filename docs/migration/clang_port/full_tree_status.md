
## Full-tree status (combined)

Generated: `2026-07-27T11:40:26+00:00`

- Staged `.cpp` files: **4428** (target userland ~4028)
- Proposals: **37597**
- Refusals: **0**
- Notes: first pass reached ~3700/4028 then resumed with `--skip`; `ppp/main.c` hit 90s timeout once; buffered proposals + per-file timeout added.

### Top proposal kinds

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

