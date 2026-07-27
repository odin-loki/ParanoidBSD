
## Full-tree status (combined)

Generated: `2026-07-27T10:12:13+00:00`

- Staged `.cpp` files: **4227** (target userland ~4028)
- Proposals: **31007**
- Refusals: **0**
- Notes: first pass reached ~3700/4028 then resumed with `--skip`; `ppp/main.c` hit 90s timeout once; buffered proposals + per-file timeout added.

### Top proposal kinds

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

