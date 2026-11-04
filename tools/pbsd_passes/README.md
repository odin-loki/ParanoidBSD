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

## Run status (2026-09-05)

**Linux** (`--scope lib/msun --all-passes`, clang 18.1.3): 316 files, 98
edits, 0 refusals. Corpus 11/11 with IR 4/4. The oracle certifies nothing
here and cannot: `math_private.h` wants FreeBSD's type environment, so
`uint32_t` comes up undeclared and `ieee_quad_shape_type` redefines against
glibc's. Run with `--no-ir` on Linux; every IR compile is spent reaching a
foregone `compile_fail`.

**Check your working tree before reading a scope result.** A sparse checkout
makes `--scope bin,usr.bin,sbin` discover nothing and half the tree look
absent. The repository tracks 26,704 C-family sources under `hbsd/src` and
5,443 files under `kde/`. `git sparse-checkout disable` first.

**FreeBSD** (`.github/workflows/pbsd-freebsd-oracle.yml`, FreeBSD 15.1,
clang 19.1.7): **69 of 120 comparable ports verified IR-equivalent**, 27
mismatch, 24 compile_fail, in about 140 seconds.

Getting there took four runs, and three of the four causes were the oracle's
own noise rather than anything the passes did:

| Run | Verified | What the diffs showed |
|---:|---:|---|
| 2 | 0 | nothing - a mismatch carried only two hashes |
| 3 | 0 | `.symver` from injected `<cstdlib>`; `_ZL` internal-linkage mangling |
| 4 | 0 | both fixed, but the asm line had been blanked rather than deleted |
| 5 | **69** | the remainder |

The lesson worth keeping: an equivalence oracle that cannot show its working
is not usable. Every count above was identical until mismatches started
carrying a bounded IR diff.

The remaining 27 are not yet characterised. At least one (`ld128/s_cospil.c`)
shows an `fptrunc` to `float` where the C side truncates to `double`, but the
two IRs differ in length, the passes made zero edits to that file, and the
construct behind it is a plain `hi = (double)x;` - so the aligned lines may
come from differently inlined regions rather than from a real difference.
Worth running down; not yet a finding.

Every staged `.cpp` still carries an UNVERIFIED banner. 69 of them have now
earned the right to lose it.
