# PBSD automated port passes (`docs/plans/todo-passes.md`)

Deterministic rewriting + refusal logging. Refusals are the model work queue.

## Run

```bash
cd "/path/to/Operating System"
python3 tools/run_todo_passes.py --corpus-only
python3 tools/run_todo_passes.py --scope bin --limit 80
python3 tools/run_todo_passes.py --scope bin,usr.bin,sbin --limit 500 --all-passes
python3 tools/run_todo_passes.py --file hbsd/src/usr.bin/true/true.c
```

WSL recommended (Clang 18).

## Outputs

| Path | Meaning |
|---|---|
| `docs/migration/clang_port/staged/**/*.cpp` | Rewritten C++23 candidates |
| `docs/migration/clang_port/refusals.jsonl` | Refusal log (schema in docs/plans/todo-passes.md) |
| `docs/migration/clang_port/pass_report.json` | Full machine report |
| `docs/migration/clang_port/todo_pass_report.md` | Human summary + reason histogram |
| `docs/migration/clang_port/compile_commands.json` | Synthetic DB for the run scope |

## Design rules

- Precondition fails → log + continue (never guess, never block).
- Tier 1 rewrites aggressively where safe; ambiguous constructs are refused.
- Tier 2/3 mostly infer + log; only high-confidence local RAII / `std::array` rewrite.
- IR oracle + differential run on small non-header-heavy TUs (`--no-ir` / `--no-diff` to skip).
