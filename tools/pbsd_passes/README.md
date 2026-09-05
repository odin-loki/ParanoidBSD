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

## Run status (2026-09-05, clang 18.1.3)

`python3 tools/run_todo_passes.py --scope lib/msun --all-passes` — 316 files,
98 edits, 0 refusals. The golden corpus passes 11/11 with the IR oracle equal
4/4, and the oracle now gates that verdict rather than only reporting it.

**Check your working tree before reading a scope result.** A sparse checkout
that materialises only `hbsd/src/lib/msun/src` and `hbsd/src/sys/sys` makes
`--scope bin,usr.bin,sbin` discover nothing and makes half the tree look
absent. It is not: the repository tracks 26,704 C-family sources under
`hbsd/src` and 5,443 files under `kde/`. `git sparse-checkout disable` first.

The oracle still does not verify real msun sources: 25 files are eligible and
all 25 stop at `compile_fail`. What it took to get there is worth recording,
because three of the four causes were the oracle's own include environment,
not the passes:

| Was | Cause | Now |
|---|---|---|
| `sys/_types.h`, `math_private.h` not found | oracle passed only `-Wno-everything`; the staged `.cpp` is compiled away from its siblings | fixed — source dir, `hbsd/src/sys`, `lib/libc/include` |
| `machine/_types.h` not found (20) | FreeBSD symlinks `machine/` to `sys/<arch>/include`; a plain checkout has no such link | fixed — shim dir built per process |
| `invtrig.h`, `_fpmath.h`, `IEEEl2bits` | msun's long-double helpers and the per-arch libc headers were not on the path | fixed — `msun/ld80`, `msun/ld128`, `libc/<arch>` |
| `no member named 'atoll' in namespace '__gnu_cxx'` (17) | `-I` put FreeBSD's `stdlib.h` ahead of glibc's, so libstdc++'s `<cstdlib>` picked up the wrong one | fixed — `-idirafter`, so glibc wins the standard headers |

What remains is not a path problem. `math_private.h` wants FreeBSD's type
environment (`uint32_t` undeclared, `union ieee_quad_shape_type` redefined
against glibc's), which is the same FreeBSD-on-Linux wall that keeps 148
modules in `docs/migration/linux_build_exceptions.txt`. Verifying these ports
wants a FreeBSD runner, not more `-I` flags.

Until then, treat every staged `.cpp` as UNVERIFIED — which is exactly what
the banner each one carries says.
