# Evidence for sweep 2026-09-18

| File | Size | Contents |
|---|---|---|
| `pbsd-sweep.tar.gz` | 6.5 MB | C-tree stage JSONL + `state.json`. Not the matrix. |
| `pbsd-sweep-digest.json` | 47 KB | Counts and a 400-line FAILED sample. Cannot be re-analysed. |
| `cxx-sweep.tar.gz` | 66 KB | KDE `cxx_analyze.jsonl` + `cxx_tidy.jsonl` + `state.json`. |
| `cxx-sweep-digest.json` | 2 KB | Same, smaller. |

Unpack, then rebuild the matrix from the JSONL the pack kept:

```
python3 tools/verify/pack_results.py ~/pbsd-sweep --pack pbsd-sweep.tar.gz --digest pbsd-sweep-digest.json
```

That command created these files. The tarball itself carries `sweep/HOW-TO-REBUILD.json`.
