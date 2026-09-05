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

## Run status on this checkout (2026-09-05, clang 18.1.3)

`python3 tools/run_todo_passes.py --scope lib --all-passes` — 255 files, 51
edits, 0 refusals. `--scope bin,usr.bin,sbin` discovers nothing: only `lib`
and `sys` are vendored under `hbsd/src`, and every `.c` in the checkout is
under `hbsd/src/lib/msun/src`.

The golden corpus passes 11/11 with the IR oracle equal 4/4, and the oracle
now gates that verdict rather than only reporting it.

The oracle does **not** yet verify real msun sources: 25 files are eligible
and all 25 stop at `compile_fail`, none of it the passes' doing.

| Blocker | Files | Fix |
|---|---:|---|
| `machine/_types.h` not found | 20 | vendor `sys/<arch>/include` and expose it as `machine/` |
| `invtrig.h` not found | 3 | vendor `lib/msun/ld80` (and `ld128`) |
| glibc `<complex.h>` is C-only | 2 | inherent to `_Complex` sources under C++ |

The first two are missing upstream directories, not defects here. Vendoring
them would put roughly 23 of the 25 within reach of real IR verification,
which is the difference between the mechanical phase producing candidates and
producing candidates that have been *checked*.

Until then, treat every staged `.cpp` as UNVERIFIED — which is exactly what
the banner each one carries says.
