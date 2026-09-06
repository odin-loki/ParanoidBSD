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

### The configuration matters as much as the headers

`opt_shim()` used to write every `opt_*.h` **empty**, and argued that this
was the conservative choice: code under an unset option is not checked
rather than checked wrongly. That is a good argument for one option. It is
a bad one for all of them at once, because the result is a configuration
nobody ships — with everything off, `INET` is off, and a sweep of
`sys/netinet` is checking a TCP stack compiled without IP.

Measured on `sys/netinet/tcp_syncache.c`: **2** findings with nothing
defined, **1** with `-DINET`, **7** with `-DINET -DINET6`. The third is the
only one where most of the file is compiled at all. The empty shim was not
producing conservative results; it was producing results about a different
program.

So the options come from the kernel configuration PBSD builds, read the way
`config(8)` reads it — `sys/amd64/conf/HARDENEDBSD`, following `include`,
`options FOO`, `options FOO=v` and `nooptions`, mapped through
`sys/conf/options` to the header each one lands in, with `opt_dontuse.h`
skipped because that is `config(8)`'s sink for options that only steer
`files` rules. `opt_global.h` is force-included, as `kern.pre.mk` does.

Four options are dropped, and each has to meet one test: **it adds
instrumentation, not behaviour.** `KDTRACE_HOOKS` and `KDTRACE_MIB_SDT`
turn on the SDT probes, and `sys/sys/sdt.h:218` writes them as
`asm goto(...)`, which clang's analyser gives up on — with them on,
`sys/netinet` was 85 errors out of 105 translation units and 39 of those
were that one macro. A dtrace probe does not change what the surrounding
code computes.

    sys/netinet, everything off        20 of 105 usable
    ... with KDTRACE_HOOKS on          20 of 105, 85 errors
    ... HARDENEDBSD minus the probes  100 of 105, 54 findings

`PBSD_KERNCONF=none` restores the empty shim, and `PBSD_KERNCONF=GENERIC`
or any other config under `sys/<arch>/conf` selects that one instead — a
real handle now that the answer depends on it.

### The architecture that could not be seen, and why

`sys/arm` was 1 usable translation unit out of 322. 188 of them stopped at

    machine/pcpu_aux.h:46: _Static_assert(PAGE_SIZE % sizeof(struct pcpu)
                           == 0, "fix pcpu size")
    note: expression evaluates to '256 == 0'

and the investigation went in circles for a long time: `sizeof(struct
pcpu)` came out 640 in the real translation unit and 256 in a probe that
included the same headers, while `clang -E -P` on both produced a
**byte-identical** struct — 1030 characters, no diff. Two textually
identical structs cannot have different sizes, so one number was not
measuring what it appeared to.

`-Xclang -fdump-record-layouts` said which:

    0 | struct pcpu
    0 |   struct thread * pc_curthread
    8 |   struct thread * pc_idlethread

Eight-byte pointers, on a 32-bit architecture. **Nothing in this tooling
ever passed `--target`**, so clang used the HOST triple for all six
architectures, and the probe's 256 was as wrong as the file's 640 — both
were x86-64.

    no --target   sizeof(struct pcpu)=640   4096%640=256   rc=1
    armv7 triple  sizeof(struct pcpu)=512   4096%512=0     rc=0

The assertion the tree ships is correct and always was. arm and i386 are
ILP32 and were being compiled LP64 — every struct layout, every pointer,
every `long`. arm64, riscv64 and powerpc64 are LP64 like the host, so they
only *accidentally* agreed; powerpc64 is big-endian and was being checked
little-endian. Only amd64 was ever right.

`TRIPLE` in `includes.py` fixes it for clang. goto-cc is a gcc driver: it
rejects `--target=`, `--arm-linux` is an "uninterpreted gcc option", and
`-m32` is the one thing it takes — so the model checker gets i386 exactly,
armv7 in every respect CBMC reasons about, and the host model for the
rest. **powerpc64's endianness cannot be modelled by goto-cc at all**, and
endianness-dependent CBMC results on powerpc64 are not sound. That is the
compiler's limit, and it is stated rather than papered over.

    sys/arm     1 of 322 -> 274 of 323
    sys/i386             -> 46 of 52

The moral is the one this file keeps relearning: when two measurements of
the same thing disagree, at least one instrument is lying, and the answer
is to measure the instrument.

### What the tree says about a file, read instead of guessed

`sys/conf/files*` carries `compile-with` for 402 sources —

    contrib/ck/src/ck_epoch.c standard compile-with "${NORMAL_C} -I$S/contrib/ck/include"

— so `conf_file_includes()` reads it the way `kernconf_options()` reads
`sys/conf/options`. `MODULE_INCLUDES` adds the sets that live only in
module Makefiles, each entry citing the line it came from:
`LINUXKPI_INCLUDES` (`kmod.mk:114-117`), `CDDL_CFLAGS`/`ZFS_CFLAGS`
(`kern.pre.mk:172-208`), libsodium (`files:682`), and the
`-DKBUILD_MODNAME` every `contrib/dev` driver names itself with.

Order is not cosmetic. `kern.pre.mk` puts openzfs's spl include directory
ahead of `-I$S` and passes `-D_SYS_CONDVAR_H_` so `<sys/condvar.h>`
resolves to openzfs's. Append it *after* `-I sys` instead and the guard
silences FreeBSD's header while nothing supplies the replacement, so
`struct cv p_pwait` at `sys/sys/proc.h:775` has incomplete type — a
compile error invented entirely by flag order.

### Not everything under `sys/` is a kernel translation unit

`includes.NOT_KERNEL` lists the subtrees that are userland C living under
`sys/`: ACPICA's host tools (`iasl`, `acpidump`), OpenZFS's upstream Linux
userland (`cmd/`, `tests/`, `udev/`, …) and its Linux kernel port. "Wants
`<stdio.h>` under `-D_KERNEL`" is not a finding about the kernel, and 700
of them drowned the ones that were.

Both instruments consult it, so they agree on what is being checked.

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

`sys/netinet6/mld6.c` made the same point more sharply. `sys/netinet/igmp.c`
and `mld6.c` carry byte-for-byte the same division by zero — the same
`min()` with a timer field that means *stopped* when it is 0. The analyser
found it in `igmp.c` and reported **zero findings** for `mld6.c`, which
compiled cleanly. So: read the class, then grep for it.

So: `M_WAITOK` cannot fail, `M_NOWAIT` can and returns NULL, the difference
is one token, and a pattern check finds every place the second is written
and the result used anyway.

It **gates** now. It did not, and the reason was in its docstring: four
false-positive classes, each found by reading output that looked like a
hundred bugs and was not — `if ((p = malloc(...)) == NULL)` puts the test
nowhere near the variable name; `if (m)` is a NULL test; `sizeof(p->x)`
does not evaluate `p`; `mhead = mtail = alloc()` is checked on the other
name. It also read its own explanatory comment as a bug.

Those four are handled, the nine real sites it found are fixed, and the
tree is at zero — so `--gate` is about the *next* one. A fifth class gets
fixed by teaching `sites()` about it, never by an allowlist: an allowlist
here would hide the next real one, which is the exact failure this lint
exists to prevent.

### A fourth instrument, from the same lesson

`tools/verify/masked_switch_check.py` exists because `procctl(2)` had
this, twice:

```c
int d;

switch (p->p_flag2 & (P2_ASLR_ENABLE | P2_ASLR_DISABLE)) {
case 0:                d = PROC_ASLR_NOFORCE;       break;
case P2_ASLR_ENABLE:   d = PROC_ASLR_FORCE_ENABLE;  break;
case P2_ASLR_DISABLE:  d = PROC_ASLR_FORCE_DISABLE; break;
}
...
*(int *)data = d;
```

Two bits is four values; there are three arms. `ENABLE|DISABLE` falls
through with `d` uninitialised and `*(int *)data` copies it to
userland — a kernel stack disclosure rather than a wrong answer.
`-Wuninitialized` says nothing about it, and the analyser found it in
one file the same way it found `igmp.c` and missed `mld6.c`.

So this looks for the *shape*: a `switch` over `X & (A | B | …)`, with
no `default:`, fewer than `2**N` arms, and a variable assigned inside
that was declared without an initialiser. That last condition is what
separates "the author enumerated the states that matter" from "a value
escapes uninitialised", and it takes 37 candidate switches under `sys/`
down to the two that mattered.

Its own first version reported line 724 for a defect on line 820,
because stripping `/* … */` **deleted** the lines rather than blanking
them. A checker that names the wrong line is worse than one that names
none — and it is the third tool here to have been caught out by its own
explanatory comment.

### And the compiler cannot substitute for the analyser

The five uninitialised returns in `docs/security/UB_FINDINGS.md` are all
"assigned only inside a loop that some path does not enter". That looks
like something `-Wuninitialized` should say, and it is not — not for the
real files, and not even for

```c
int f(int n) { int e; for (int i = 0; i < n; i++) e = i; return e; }
```

which `-Wuninitialized`, `-Wsometimes-uninitialized` and
`-Wconditional-uninitialized` all compile in silence. clang's flow-sensitive
warnings are deliberately conservative; the path-sensitive analyser is not.
That is what `--analyze` buys over a warning flag, measured rather than
assumed.

### What "interprocedural" does and does not mean

The analyser is interprocedural **within** a translation unit and not
**across** one, and the difference accounts for most of `sys/geom`'s
findings in one go. 34 GEOM files carry

```c
buf = g_read_data(cp, offset, length, &error);
if (buf == NULL)
        return (error);
/* decode into *md */
return (0);
```

and a caller that tests `error != 0` before reading `md`.
`g_read_data()` returns NULL exactly when it has set `*error` —
`if (errorc) { g_free(ptr); ptr = NULL; }` is the last thing it does and
there is no earlier return — so the struct is never read untouched. But
`g_read_data()` lives in `sys/geom/geom_io.c`, a different translation
unit, so the analyser must assume it may return NULL and leave `*error`
alone; every field of `md` is then a garbage value and every comparison
against it is a finding.

Nothing in the tree is wrong and nothing in the analyser is wrong. The
lesson is that a finding in a caller can be a fact about a **callee it
could not see**, and that one such contract, copied 34 times, looks like
a class of defects.

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

Each was confirmed with UBSan before anything was edited, and re-checked
after: exhaustively against an independent reference for `stdbit`
(197,376 cases, zero mismatches), and `PROVED` by CBMC plus an identical
bit pattern for `rint`.

That table was the whole list once. **`docs/security/UB_FINDINGS.md` is
the list now** — around thirty defects, and, just as usefully, the ones
that looked like defects and were not, each with the reasoning that
killed it. The reasoning is the part worth keeping: it is what stops a
finding being re-reported, and three plausible ones died that way
(`FP_ILOGB0` is `-INT_MAX` and not `INT_MIN`, so `-ilogb(x)` cannot
overflow).

The high-water marks so far, all reachable without privilege or from the
network:

- `mincore(addr, 0, vec)` returns an uninitialised `int` from the kernel;
- a peer's RPC reply could `abort()` any client, because libc is not
  built `-DNDEBUG`;
- a user-supplied netlink nexthop weight of 0 divided by zero in the
  kernel;
- `snl_free()` was not idempotent, and closed the same fd twice.
