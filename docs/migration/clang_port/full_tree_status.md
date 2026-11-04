
## Full-tree status (combined)

Generated: `2026-07-27T11:59:57+00:00`

- Staged `.cpp` files: **4428** (target userland ~4028)
- Proposals: **31306**
- Refusals: **0**
- Notes: first pass reached ~3700/4028 then resumed with `--skip`; `ppp/main.c` hit 90s timeout once; buffered proposals + per-file timeout added.

### Top proposal kinds

| Kind | Count |
|---|---:|
| `MACRO_OBJECT` | 7680 |
| `STR_FORMAT_CANDIDATE` | 3631 |
| `MALLOC_ESCAPE` | 2579 |
| `SPAN_CANDIDATE` | 2334 |
| `NULLABILITY` | 2124 |
| `LIFETIME_OWN` | 1606 |
| `ANDERSEN_OWNED` | 1599 |
| `LIFETIME_FAIL` | 1435 |
| `GLOBAL_CLUSTER` | 1227 |
| `MACRO_CONSTEXPR` | 1192 |
| `POINTER_KIND` | 1156 |
| `ANDERSEN_ESCAPE` | 962 |
| `MACRO_DIVERGENT` | 776 |
| `DEAD_STORE` | 632 |
| `SYSCALL_TRACE` | 609 |
| `PURITY` | 454 |
| `KR_DEFINITION` | 247 |
| `VLA` | 234 |
| `GOTO_CLEANUP_CANDIDATE` | 233 |
| `QUEUE_H_SITE` | 163 |
| `TYPEDEF_COMPLEX` | 128 |
| `CALL_SITE_SPAN` | 83 |
| `RANGE_FOR_CANDIDATE` | 82 |
| `CALLBACK_CTX` | 47 |
| `GOTO_CROSS_INIT` | 29 |

