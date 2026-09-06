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

The kernel needed three more, each found by reading what the failures
actually said rather than the total:

- **The interface headers are generated.** 2,540 of the 6,345 kernel
  translation units the sweep could not compile failed on
  `fatal error: 'device_if.h' file not found`, and 369 more on
  `vnode_if.h`. Those files do not exist in a source tree: `buildkernel`
  runs `awk -f sys/tools/makeobjops.awk sys/kern/device_if.m -h` over each
  of the 138 `*_if.m` interface descriptions, and `vnode_if.awk` over
  `sys/kern/vnode_if.src`. `iface_shim()` runs the same generators on the
  same inputs, so the declarations are the ones the kernel compiles
  against — not a stub.
- **A module's own subsystem root.** `sys/amd64/vmm/io/ppt.c` includes
  `vmm_lapic.h`, a sibling one level up, and `io/iommu.h`, named relative
  to `sys/amd64/vmm` rather than to the file. The vmm module's Makefile
  adds `-I${SRCTOP}/sys/amd64/vmm`; `_subsystem_dirs()` adds every
  directory between `sys/` and the file, nearest first.
- **The preprocessor was answering Linux.** clang and goto-cc take their
  platform macros from the host triple, so every translation unit in this
  sweep saw `__linux__ 1` and no `__FreeBSD__`. 607 files under `sys/`,
  `lib/libc` and `lib/msun` key on one of those two names — libsodium,
  ck, ACPICA, linuxkpi, zfs. Every `#ifdef __FreeBSD__` took its false
  branch and every `#ifdef __linux__` its true one, so in those files the
  checked code was not the code that ships. `-U__linux__ -U__gnu_linux__
  -D__FreeBSD__=15` fixes the identity; `--target=` is not used because
  goto-cc is a gcc driver and does not accept it.

Measured: `sys/amd64/vmm` **12 → 26 of 34**, `sys/kern` **67 → 78 of 212**.
The gain is largest where drivers are, because that is where `device_if.h`
is.

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

## Why there is a third instrument

`tools/verify/nowait_check.py` is a lint, and it exists because of one
result. clang's analyser reported

    sys/netgraph/netflow/ng_netflow.c:414  Attempt to free released memory

and the same file has the identical bug at `:388` and `:556`. It reported
neither. The analyser explores paths and stops at the first defect on each,
so **three instances of one mistake in one file came back as one finding**.
Exhaustiveness over paths is not exhaustiveness over instances.

So: `M_WAITOK` cannot fail, `M_NOWAIT` can and returns NULL, the difference
is one token, and a pattern check finds every place the second is written
and the result used anyway. Nine of the nine it reports today are real.

It **reports and does not gate**, and the reason is in its docstring: four
false-positive classes, each found by reading output that looked like a
hundred bugs and was not — `if ((p = malloc(...)) == NULL)` puts the test
nowhere near the variable name; `if (m)` is a NULL test; `sizeof(p->x)`
does not evaluate `p`; `mhead = mtail = alloc()` is checked on the other
name. It also read its own explanatory comment as a bug. A lint that cannot
parse C cannot carry a build.

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

### `static` and exported are different claims

A modular check explores a function's **whole signature domain**. Whether
that is the right domain depends on linkage, which the goto symbol table
records as `file_local`:

| | |
|---|---|
| **exported** | the signature domain *is* the contract callers are entitled to, so UB anywhere in it is a defect. `stdc_leading_ones_uc(unsigned char)` is UB for every `x >= 128`; `rint(double)` for every negative `x`. Both were real. |
| **`static`** | the wrong domain — the only callers are in the same file and they constrain it. `nsap_addr.c`'s `xtob(int c)` subtracts `'0'` or `'7'` and overflows at `INT_MIN`; its one caller passes an `isxdigit()`-validated char. |

A `static` finding is not dismissed, it is **deferred**: proving it
unreachable needs the callers, which a modular check does not have.

### Extern function returns are unconstrained too

`clock()` overflows on `CONVTCK(ru.ru_utime) + CONVTCK(ru.ru_stime)` — but
`ru` was filled by `getrusage()`, which CBMC does not model, so its fields
are arbitrary. Same for `alarm()` on `setitimer`'s `oitv`, and `svc_run()`.
This is the pointer-parameter trap a third time: unconstrained input
reported as a defect. `nice(int incr)` is the one in that group worth a
second look, because `incr` is a **parameter** of an exported function, so
`nice(INT_MAX)` really does overflow `prio + incr`.

### `division by zero` needs triage, in both directions

`--div-by-zero-check` does not distinguish integer from floating-point
division, and only the integer one is undefined:

```
$ cbmc fdiv.c --function fdiv --div-by-zero-check     /* double a / b */
[fdiv.division-by-zero.1] division by zero in a / b: FAILURE
```

IEEE-754 defines `x / 0.0` as ±∞ with a flag raised, and `lib/msun`
**relies on that** — it is how `log(0)`, `logb`, `rsqrt` and the `catrig`
family produce their infinities. Thirty-odd of the first run's "failures"
in `lib/msun` were exactly this and not one was a defect.

So a `division by zero` finding is read by hand: integer, and it is real;
floating-point, and it is the library working as designed. The tool does
not guess, and it does not silently drop the class either — an integer
division by zero in `lib/libc/stdlib/getopt_long.c:gcd` is in the same
list and is worth looking at.

## Transfer to PBSD

For a port the IR oracle certifies `ir.equal` **and** `abi_equal`, the C
and the C++ compile to identical LLVM IR. Any property proved of the C
therefore holds of the C++ without re-verification. CBMC's C++ front end
cannot parse the ports anyway (`parse error before 'noexcept'`), and it
does not need to: a pure rename is byte-identical, so it is checked as C
and the certificate carries the result across.


## What it has found

| | |
|---|---|
| **`lib/libc/stdbit`** — six C23 functions | `x << offset` on a promoted `unsigned char`/`unsigned short`. UB for any `x` with the top bit set. `stdc_first_leading_zero.c`, same author, has the `(unsigned int)` cast the other three omit. |
| **`lib/msun/src/s_rint.c`, `s_rintf.c`** | `sx << 31` where `int32_t sx = (i0>>31)&1`. `1 << 31` on a signed int is UB, and `rint()` reaches it on **every negative argument**. |
| **`lib/msun/src/s_ceil.c`, `s_floor.c`** | `i1 + (1<<(52-j0))` in the `j0` ∈ [21,51] branch. At `j0 == 21` that is `1 << 31` on a signed `int`. `ceil(3000000.5)` reaches it. |
| **`lib/libc/gen/nice.c`** | `prio + incr` — `incr` is the caller's `int`, so `nice(INT_MAX)` overflows. Reported, not yet fixed: defining it changes an exported function's behaviour and that is a decision, not a repair. |

Both were confirmed with UBSan before anything was edited, and both were
re-checked after: exhaustively against an independent reference for
`stdbit` (197,376 cases, zero mismatches), and `PROVED` by CBMC plus an
identical bit pattern for `rint`.
