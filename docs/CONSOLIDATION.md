# Six architectures, six implementations — what can become one?

Asked whether six per-architecture implementations can be consolidated into
C++ and modules. Mostly yes, but the reason a thing is duplicated decides
whether it can, and there are two different reasons that look the same in a
file listing.

## The two kinds of duplication

Measured over `sys/<arch>/include`, 30 headers exist in three or more
architectures, 14,098 lines in total:

| lines | similarity | arches | header |
|---:|---:|---:|---|
| 2,062 | **0.01** | 6 | `atomic.h` |
| 1,302 | 0.12 | 4 | `bus.h` |
| 940 | 0.05 | 6 | `pmap.h` |
| 864 | 0.02 | 6 | `cpufunc.h` |
| 721 | 0.01 | 6 | `cpu.h` |
| 637 | **0.44** | 4 | `_inttypes.h` |
| 626 | 0.00 | 5 | `pte.h` |
| 323 | **0.53** | 4 | `_stdint.h` |

**High similarity is copy-paste.** `_stdint.h` at 0.53 and `_inttypes.h` at
0.44 are the same file edited four times. Deduplicating them is mechanical.

**Low similarity is the interesting case, and it is the opposite of what it
looks like.** `atomic.h` scores 0.01 — six texts with almost nothing in
common — and they are six implementations of one small set of operations.
That is a stronger case for consolidation than copy-paste is, not a weaker
one, and a similarity metric scores it lowest of anything in the table.

That is the flaw in `arch_duplication.py`: it finds copies, and the biggest
prize is not a copy.

**Low similarity can also mean genuinely different semantics.** `pmap.h`,
`pte.h` and `vmparam.h` also score near zero, and there the difference is real
— page table entry layout is fixed by the MMU. Those do not consolidate at any
price.

Telling the two apart is a question about the interface, not the text: does
every architecture export the same names?

## Atomics: the strongest case

| arch | lines | inline asm sites |
|---|---:|---:|
| amd64 | 598 | 17 |
| arm64 | 679 | 18 |
| arm | 1,111 | 25 |
| i386 | 874 | 19 |
| powerpc | 1,183 | 42 |
| riscv | 662 | 24 |
| **total** | **5,107** | **145** |

The interfaces are **not** identical, and checking that is what makes the case
rather than weakens it. Each architecture defines between 122 and 204
`atomic_*` names; the union is 247 and only **43 are common to all six**.

Decomposed, those 247 names are a cross-product:

| | count | |
|---|---:|---|
| operations | 13 | `add`, `subtract`, `set`, `clear`, `cmpset`, `fcmpset`, `load`, `store`, `swap`, `fetchadd`, `readandclear`, `testandset`, `testandclear`, plus `thread_fence` |
| widths | 9 | `8`, `16`, `32`, `64`, `char`, `short`, `int`, `long`, `ptr` |
| orderings | 4 | relaxed, `acq`, `rel`, `acq_rel` |

A complete cross-product is 936 names. 247 exist, spread unevenly across six
files, because each architecture wrote out the subset it happened to need.

Those are the names written out **literally**. Three architectures also
generate names with token-pasting macros — arm64 has 18 such generators,
powerpc 11 — so `atomic_add_32` exists on arm64 without the string
`atomic_add_32` appearing anywhere in its header.

That matters for what can be claimed. A first attempt at a parity check read
the headers literally and reported 50 atomics "used by machine-independent
code but missing on some architecture". Most were macro-generated and present.
**Which cells of the cross-product are actually filled cannot be read off the
source** — it needs the preprocessor with each architecture's real include
path, which means running on FreeBSD. `tools/atomic_survey.py` reports what is
countable and says so about the rest.

What survives that correction is the burden itself: a regular
operation × width × ordering space, filled in by hand, unevenly, six times,
partly literally and partly by six different sets of macros. That is the
argument for generating it once, and it does not depend on knowing which cells
are missing.

A template generates the cross-product uniformly and completely. Clang already
knows how to emit the right instruction for every one of those targets — that is what `__atomic_*` builtins and `std::atomic` are. A generic
implementation is a few hundred lines and the compiler supplies the per-target
codegen that is currently written by hand.

This is also where the payoff for a seventh architecture is largest: today it
costs another ~850 lines of correct lock-free assembly — the mean across the
six — which is the hardest code in a port to get right and the worst to debug
when it is subtly wrong.

Note there is **no atomic assembly in `.S` files at all** — 0 of 214 eliminable
files. Atomics live in headers as inline asm, so "replace hand-written atomics
with `std::atomic`" is a different job from the assembly removal, and a bigger
one.

## The full list, with what replaces what

| what | size | replaced by |
|---|---:|---|
| six `atomic.h` | 5,107 lines | `__atomic` builtins — **written, see below** |
| eliminable `.S` | 28,820 lines | generic C already present, via `MK_MACHDEP_OPTIMIZATIONS` |
| `_stdint.h`, `_inttypes.h` | 960 lines, 4-5 copies | `<cstdint>` is literally this |
| `stand/ficl/<arch>/sysdep.{c,h}` | 810 duplicated lines, 6 arches, 0.95 similar | one implementation |
| `lib/libc/<arch>/softfloat/softfloat.h` | 333 duplicated lines | one `basic_float<E,M>` template — **written, see docs/FLOAT.md** |
| `sys/<arch>/include/bus_dma.h` | 120 duplicated lines | one interface |

## What does not consolidate, and why

- **`pmap.h`, `pte.h`, `vmparam.h`, `cpufunc.h`** — the MMU and the
  instruction set differ. This is the irreducible 41,381 lines by another
  route.
- **`linux_proto.h`, `linux_sysent.c`, `linux_systrace_args.c`,
  `linux_syscalls.c`** — 8,310 duplicated lines across three architectures,
  and all of it generated from `syscalls.master`. The duplication is in the
  output. Consolidating it means changing the generator, and the generator is
  not the problem.
- **`sys/contrib/ck`** — vendored concurrency kit, upstream's to deduplicate.

## Modules, specifically

C++23 modules help the *build*, not the duplication: one `pbsd.atomic`
interface with per-target partitions is a cleaner shape than six headers, but
the reason six headers collapse into one is `__atomic` builtins, not modules.
Modules are how the result is packaged; builtins are why there is a result.

Worth being precise about that, because "port it to modules" and "stop writing
six implementations" are separable, and only the second reduces work.


## Generic atomics: written, and measured

`hbsd/src/sys/sys/atomic_generic.h`, 350 lines, is the whole of atomic(9) —
13 operations across 4 widths and 4 orderings, plus the char/short/int/long/
ptr spellings — expressed in `__atomic` builtins. It replaces, in principle,
5,107 lines across six headers with 145 inline-asm sites.

"In principle" is doing work in that sentence, so it was measured rather than
argued. `tools/atomic_generic_check.py` compiles the header for all six
targets against a shimmed `<sys/types.h>` and nothing else on the include
path, one object per width, and reads the undefined symbols back out. A
`__atomic_fetch_add_8` in the undefined list means the backend could not do it
in an instruction and emitted a call into libatomic — which the kernel does
not link, so that is a link error waiting, not a slow path.

```
arch               8         16         32         64   char/short/int/long/ptr
amd64      lock-free  lock-free  lock-free  lock-free   ok
arm64      lock-free  lock-free  lock-free  lock-free   ok
arm        lock-free  lock-free  lock-free  lock-free   ok
i386       lock-free  lock-free  lock-free    libcall   ok
powerpc    lock-free  lock-free  lock-free  lock-free   ok
riscv      lock-free  lock-free  lock-free  lock-free   ok
```

Twenty-three of the twenty-four cells come out in hardware, armv7's 64-bit
included — AAPCS aligns 64-bit types to 8, so LDREXD/STREXD is available to
the compiler with nobody asserting anything.

The twenty-fourth is the interesting one. **i386 64-bit is not lock-free**,
and not for the reason it looks like: the instruction exists. The i386 ABI
aligns `uint64_t` to 4, and `__atomic` will not assume the 8 that CMPXCHG8B
wants, so it calls libatomic instead. Asserting the alignment does produce
CMPXCHG8B — checked, two of them — but that is a promise about the caller's
object which a header cannot make, and clang says so under
`-Walign-mismatch`. `-march=i586`, `i686` and `pentium4` change nothing; it
was never about the instruction set.

`sys/i386/include/atomic.h` makes that promise for itself and can keep its
own 64-bit block. So the adoption is not all-or-nothing, and the shape of it
is now a fact rather than a guess:

* every architecture takes 8, 16 and 32 bits from the generic header;
* every architecture except i386 takes 64 as well;
* i386 keeps 44 lines of 64-bit operations out of its 874.

The checker was made to fail before being trusted: marking i386's 64-bit
width as required reports all eight libcalls by name, and a syntax error in
the header is caught as a compile failure rather than passed over.

**Nothing includes it yet.** The switch-over is a separate change and wants a
green `buildworld` behind it, for the reason `docs/migration/
COMMITTING_PORTS.md` gives about the msun ports: while a build failure could
still be the tree rather than the change, it cannot be attributed.

## `_stdint.h` and `_inttypes.h`: written, and three ABI traps found

`hbsd/src/sys/sys/_stdint_generic.h` (150 lines) and `_inttypes_generic.h`
(198 lines) replace five copies each of `<machine/_stdint.h>` and
`<machine/_inttypes.h>` — 2,041 lines — by writing every macro in terms of a
compiler predefine rather than a number.

`tools/stdint_generic_check.py` preprocesses the generic header and the
architecture's own for all six targets and compares every macro: numerically
for the limits, because `(-0x7f-1)` and `(-127 - 1)` are the same value
written differently, and textually for the conversions, after concatenating
adjacent string literals because FreeBSD writes `PRId64` as `__PRI64"d"`.

```
header         arch       macros   same  differ  missing
_stdint.h      amd64          57     57       0        0
_inttypes.h    amd64         154    154       0        0
   ... the same on arm64, arm, i386, powerpc, riscv

1266 macro expansions compared, all equal.
```

Getting there took three corrections, and each was a silent ABI change that
the checker caught and reading would not have:

1. **`int_fast8_t`.** clang's is a `signed char`, so `__INT_FAST8_MAX__` is
   127. FreeBSD's is `int32_t` — `<machine/_types.h>` says so identically on
   all six — so the limit is `0x7fffffff`. The generic header uses the
   tree's choice, which happens to be machine-independent, not the
   compiler's.

2. **`wint_t` on riscv.** clang defines `__WINT_UNSIGNED__` there, following
   the RISC-V psABI. FreeBSD's `wint_t` is `__ct_rune_t`, which is `int`,
   machine-independently. The first version asked the compiler and produced
   `WINT_MIN` as `0` on riscv and `INT32_MIN` on the other five — a
   signedness change in a standard type, on one architecture, from a header
   whose purpose was making them the same.

3. **`PRId8`.** The first `_inttypes_generic.h` used clang's per-type
   predefines for both PRI and SCN, which made `PRId8` `"hhd"`. FreeBSD says
   `"d"`, and FreeBSD is right: printf's argument has already been promoted
   to `int`. The distinction is not cosmetic in the other direction —
   `SCNd8` must be `"hhd"`, because `"d"` would write four bytes through an
   `int8_t *`.

What cannot be made generic is `SIG_ATOMIC_MIN`, `SIG_ATOMIC_MAX` and
`SIG_ATOMIC_WIDTH`. `sig_atomic_t` is 64-bit on amd64, arm64 and riscv and
32-bit on i386, arm and powerpc — powerpc64 included, so it does not follow
`__LP64__` — and clang disagrees with FreeBSD about amd64. Those three stay
in `<machine/_stdint.h>`, which is where about 180 lines per architecture
becomes 3.

The checker was made to fail before being trusted, in both files: pointing
`INT_FAST8_MAX` at `INT8_MAX` and `SCNd8` at the int conversion are each
reported by name, on every target, with both expansions shown.

**Nothing includes these yet either.** Same reason as the atomics.
