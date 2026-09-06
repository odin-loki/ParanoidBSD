# Model checking hbsd/src

Three tools, in the order they have to run.

| | |
|---|---|
| `includes.py` | compile FreeBSD source on a Linux host, against FreeBSD's own headers |
| `classify.py` | which functions can be checked soundly with no precondition |
| `cbmc_driver.py` | run CBMC, record what kind of answer each function got |

## Why `includes.py` exists

The first analyser run over this tree produced 32 diagnostics on 12 files
and every one was `typedef redefinition ... '__int32_t' (aka 'int') vs
'long'` — FreeBSD's `<sys/_types.h>` meeting glibc's. The IR oracle
survives that by passing `-Wno-everything` and comparing IR, so identical
noise cancels on both sides. Nothing cancels for a model checker: CBMC
could not build **91 of 120** translation units.

The headers are in the tree. `-nostdinc` drops glibc and it becomes
self-hosting, given three things the build normally arranges:

- `machine/`, `x86/` — symlinks to `sys/<arch>/include`.
- `include/Makefile`'s **`LHDRS`** — `errno.h`, `stdint.h`, `fcntl.h`,
  `syslog.h`, `ucontext.h`, `poll.h`, `aio.h`, `stdatomic.h`,
  `linker_set.h` are **not in `include/`**. They are symlinked from
  `sys/sys/` at install time. 293 translation units failed on `errno.h`
  alone. Parsed from the Makefile so it tracks upstream.
- clang's resource headers — `stddef.h`, `stdarg.h`, `limits.h` belong to
  the compiler, and `-nostdinc` drops those too.

Plus `math.h`/`complex.h`/`fenv.h` from `lib/msun`, `float.h` from the
arch directory, and per-file architecture: `lib/libc/aarch64/gen/*.c` gets
aarch64's `<machine/*.h>`, because checking it against amd64's is not a
check of that file.

**480 → 1000 of 1306 translation units.**

## Why `classify.py` exists

CBMC started at an arbitrary function makes that function's parameters
nondeterministic. For scalars that is exactly right. For pointers it is
not, and the first run over `lib/libc/string` said so:

```
FAILED lib/libc/string/strcat.c:strcat
    line 41 pointer arithmetic: pointer NULL in s + 1l
```

`strcat(3)` requires a valid, NUL-terminated, non-overlapping buffer. A
nondeterministic `char *` includes NULL and includes a dangling pointer,
so CBMC reports **the absence of a precondition as a defect** — on a
function that has none. Every string routine fails that way and not one of
those failures is real.

So the corpus splits, and the split is decidable from the signature taken
out of CBMC's own goto model rather than by parsing C:

| class | meaning |
|---|---|
| `SCALAR`, `VOID` | unguarded modular checking is **sound** |
| `POINTER` | needs a harness establishing the callee's precondition; the result is then only as good as that precondition, and is recorded with it |
| `OTHER` | neither — triaged by hand |

## What a result means

`cbmc_driver.py` never merges these:

| | |
|---|---|
| `PROVED` | every property SUCCESS **and** no unwinding assertion failed. The loops closed inside the bound, so this is a proof over all inputs for the checked properties. |
| `BOUNDED` | every property SUCCESS but an unwinding assertion failed. No counterexample within *K* iterations. Nothing beyond that. |
| `FAILED` | a real violation, with a counterexample. |
| `ERROR` / `NOFUNC` / `TIMEOUT` | did not compile / not in this unit / solver did not finish. |

## Two tiers of check, and why

The very first function this was ever run on, `lib/libc/string/ffs.c`,
came back `VERIFICATION FAILED` on

```
arithmetic overflow on signed to unsigned type conversion in (unsigned int)mask
```

which is not a bug — negative `int` → `unsigned int` is **defined**,
modular reduction, C17 6.3.1.3p2. `--conversion-check` is a lint, and
systems code does that deliberately. Left on it would have reported a
defect in a three-line function that has none, 114,217 times.

- **UB tier** (a failure *is* a defect): bounds, pointer, div-by-zero,
  signed overflow, undefined shift, pointer overflow, memory leak.
- **Advisory tier** (defined but suspicious, gates nothing): conversion,
  unsigned overflow, NaN, float overflow.

## Transfer to PBSD

For a port the IR oracle certifies `ir.equal` **and** `abi_equal`, the C
and the C++ compile to identical LLVM IR. Any property proved of the C
therefore holds of the C++ without re-verification. CBMC's C++ front end
cannot parse the ports anyway (`parse error before 'noexcept'`), and it
does not need to: a pure rename is byte-identical, so it is checked as C
and the certificate carries the result across.
