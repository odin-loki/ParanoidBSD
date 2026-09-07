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

**Nothing includes it yet.** The blocker that held both this and the
`_stdint.h` pair — an open boot bisection, which a tree-wide header edit
would have made unattributable — is gone: run 58 boots with PaX
enforcement on and run 59 has a login. The `_stdint.h` pair went first
anyway, because its claim is checkable without running anything: every
macro is a value, and `--baseline` compares 1,284 of them against what
the six headers expanded to before the change.

`atomic.h` had no such comparison, so one was written:
`tools/atomic_codegen_check.py` compiles the same 618 operations twice for
each of the six targets — once against `<machine/atomic.h>`, once against
`<sys/atomic_generic.h>` — at `-O2`, and compares the instructions.

Two sequences count as equal after dropping the assembler's comments,
`.cfi` directives and clang's inline-asm markers, renumbering local labels
in order of first appearance, and joining the x86 `lock` prefix to its
instruction (the machine header writes `lock ; addl` in inline asm; the
builtin emits `lock addl`). Everything else counts, and a different
barrier counts most of all.

```
arch        ops   same  differ   kinds of difference
amd64       112     82      30   barrier x1, instructions x3, length x12, generic only x14
arm64       112     12     100   LSE dispatch (machine only) x78, instructions x4, length x4, generic only x14
arm         112      6     106   barrier x26, length x42, generic only x38
i386        112     46      66   instructions x9, length x38, generic only x19
powerpc     112     14      98   barrier x22, instructions x20, length x18, generic only x38
riscv        58     11      47   barrier x25, instructions x10, length x8, generic only x4
```

**171 of 618.** The generic header is not a drop-in replacement, and the
ways it differs are each worth naming:

* **arm64 loses a runtime dispatch.** `<machine/atomic.h>` reads a global
  `lse_supported` and branches between an LSE instruction (`ldadd`,
  `cas`, `casa`) and the `ldxr`/`stxr` loop. `__atomic` emits only the
  loop. That is 78 of arm64's 112 operations, and it is a performance
  decision FreeBSD made deliberately, on the architecture where it
  matters most. Everything else on arm64 is small: the four fences differ
  in *scope* (`dmb ishld` against `dmb ld` — inner-shareable against full
  system, the generic being the narrower and the one Linux uses), and the
  four acquire loads are the same `ldar` plus a register move the inline
  asm forces.
* **the barriers are two accepted idioms, and which side uses which
  depends on the operation.** On powerpc the generic acquire *load* is
  the control-`isync` sequence (`lwz; cmpd; bne-; isync`) and the machine
  one is `lwsync`; on acquire *cmpset* it is the other way round — the
  machine header ends with `isync` and the builtin with `lwsync`. On
  riscv the machine header uses a bare `fence` — a full barrier — where
  the builtin emits exactly what the operation needs: `fence r, rw` after
  an acquire load, `lr.w.aq` for an acquire cmpset. Adopting would make
  riscv's barriers *weaker and still correct*, which is a change to argue
  for on the record, not one to make by deleting a file.
* **the generic header offers more than it replaces.** 14 to 38
  operations per architecture exist only on the generic side — `arm` and
  `powerpc` have no 8- or 16-bit atomics at all — and *nothing* is
  machine-only. Whatever the six headers implement, the generic one
  implements too.

amd64 is the closest to a straight swap and its thirty are worth reading,
because they are what "adopt it here first" would cost. Twelve are
`fcmpset` at four widths and three orderings, where the builtin skips
writing `*old` on the success path and the hand-written version writes it
unconditionally — same value either way, one branch different. Three are
`atomic_clear_8`, where the generic complements a byte (`notb %sil`) and
the machine header a word (`notl %esi`) before the same `lock andb`. One
is `atomic_thread_fence_seq_cst`: `mfence` against `lock addl $0, %gs:256`,
which is FreeBSD choosing a locked no-op over `mfence` because on much x86
hardware it is faster. The remaining fourteen are operations only the
generic header has.

So the switch-over is not blocked on writing a checker any more; it is
blocked on a decision the checker has made visible. Taking the generic
atomics as they stand costs arm64 its LSE path and changes barrier
instructions on four architectures. The shape that survives this is the
one `counter.h` already suggested: the generic header as the default and
a per-architecture override where the tree has a reason, with the reason
being one of these rows rather than an assertion.

The measurement is frozen the same way `_stdint.h`'s was, and for a
sharper reason: `docs/migration/atomic_codegen.json` holds a verdict *and
a digest of both instruction sequences* per operation. Verdicts alone were
not enough — weakening an acquire to `__ATOMIC_SEQ_CST` in the generic
header left every affected operation at "different barrier", and the first
version of the gate reported no change at all. With digests the same edit
names eleven operations and says which side moved.

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

### Adopted, and how the claim survived the adoption

All three conditions this section used to list are met, so the five
`<machine/_stdint.h>` and five `<machine/_inttypes.h>` now include the
generic headers and keep only `SIG_ATOMIC_MIN`, `SIG_ATOMIC_MAX` and
`SIG_ATOMIC_WIDTH`. 2,041 lines became 592.

1. `buildworld` green — boot-image runs 48 (`src_conf=none`), 49
   (`norelro`) and 51;
2. both checkers passing;
3. **`/sbin/init` reaching a console** — run 58 boots `HARDENEDBSD` with
   PaX enforcement on, through `/etc/rc` to local daemons, and run 59 is
   a system with a login. The thirty-seven-run bisection is closed, so a
   tree-wide header change is attributable again.

The adoption breaks the check that justified it, and that had to be dealt
with before the headers were touched rather than after. Once
`<machine/_stdint.h>` is an include of `<sys/_stdint_generic.h>`,
`stdint_generic_check.py` preprocesses one header twice and reports 1,266
equal expansions forever.

So the measurement was frozen while it was still a measurement.
`docs/migration/stdint_expansions.json` holds what the six architecture
headers expanded to *before* the change — values, not spellings — and
`--baseline` checks the tree against it on every CI run. It covers
eighteen macros the old comparison could not: the `SIG_ATOMIC_*` three
stay per-architecture, so they are absent from the generic header, so
nothing was comparing them, and they are exactly what a switch-over could
get wrong.

Finding those needed the preprocessor rather than a text scan.
`<machine/_stdint.h>` on amd64 and i386 is five lines including
`<x86/_stdint.h>`; reading the file for `#define` finds nothing, and the
table would have covered four architectures while claiming six. `clang
-dM` with the header and without, differenced, finds what it defines.

```
1284 frozen expansion(s) checked against the architecture headers as they are now.
1266 macro expansions compared, all equal.
```

One deliberate behaviour change, on three of the five. `arm`, `powerpc`
and `riscv` wrapped their limit and constant macros in `#if
!defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)`, which C++11 made
obsolete and which `x86` and `arm64` had already dropped. The generic
header has no such guard, so those macros are now defined in C++ on all
six — the majority behaviour, and the one a C++23 tree needs.

## Ranking by the interface, not the text

`docs/PORTABILITY.md` left a question open: `atomic.h` scores 0.01 text
similarity across six architectures and implements one interface, while
`pte.h` scores 0.00 and genuinely differs, and nothing could tell them apart
because similarity measures the prose. `tools/arch_interface.py` measures the
contract instead:

```
agreement = |names exported by every architecture| / |names exported by any|
```

and ranks by `agreement - similarity`. A large gap is the `atomic.h` shape:
one contract, several unrelated implementations, which is the strongest case
for writing it once. A small gap on a low agreement is a real machine
dependency.

```
 agree   sim    gap  lines  n  shared  total  gen  header
  0.67  0.10   0.57    154  3       6      9    0  iodev.h
  0.62  0.07   0.55    721  6       8     13   15  counter.h
  0.50  0.04   0.46    174  6       4      8    0  _bus.h
  0.60  0.19   0.41    371  6       6     10    0  kdb.h
  0.31  0.03   0.28    570  6      15     49    0  db_machdep.h
  0.27  0.05   0.23   1695  6      32    117    8  vmparam.h
  0.17  0.01   0.16   5107  6      44    263  197  atomic.h
  0.08  0.04   0.04   1789  6      25    302   30  pmap.h
  0.03  0.01   0.02   1529  6       7    252  175  cpu.h
```

Two new candidates, both checked by hand rather than taken from the score:

* **`counter.h`** — 721 lines across six architectures, and all six export
  the same five names: `counter_u64_add`, `counter_u64_add_protected`,
  `counter_enter`, `counter_exit`, `EARLY_COUNTER`.

  That is where the score stops and reading starts, and reading changes the
  answer. The six `counter_u64_add` bodies are **four** implementations, not
  one:

  | arch | body |
  |---|---|
  | arm64, arm, riscv | `atomic_add_64((uint64_t *)zpcpu_get(c), inc)` — identical |
  | amd64 | `zpcpu_add(c, inc)`, which is one `addq %r, %gs:(%r)` |
  | i386 | runtime `CPUID_CX8` test, then `cmpxchg8b` or a critical section |
  | powerpc | inline `ldarx`/`stdcx.` off SPRG0 |

  Three of the four differences are real and one is not. amd64's is a
  *performance* choice — per-CPU data needs no `lock` prefix, and replacing
  it with `atomic_add_64` would put one on a counter in the hottest paths in
  the kernel. i386's is the CX8 problem `tools/atomic_generic_check.py`
  found from the other direction: 64-bit atomics are not lock-free there.
  powerpc's `ldarx`/`stdcx.` is what `atomic_add_64` compiles to on powerpc,
  written out by hand.

  So the generic version covers four architectures — arm, arm64, riscv and
  powerpc — and amd64 and i386 keep an override, each for a reason the tree
  documents elsewhere. That is about 350 lines rather than 721, and it means
  a seventh architecture writes no `counter.h` at all.

  `sys/sys/pcpu.h` already has this shape one layer down: `#ifndef
  zpcpu_add_protected` with an MI fallback that amd64 overrides. The pattern
  to copy is in the tree.

  **This paragraph replaces an earlier version of itself** which said "the
  add is exactly what `sys/sys/atomic_generic.h` now provides". For three of
  six it is; for amd64 it would have been a `lock` prefix on a hot counter.
  That is what taking an agreement score for an answer looks like, two
  sections after a tool was added to stop exactly that.
* **`_bus.h`** — 174 lines across six for four typedefs, `bus_addr_t`,
  `bus_size_t`, `bus_space_tag_t` and `bus_space_handle_t`, identical in
  every architecture except for the width behind them. That is
  `<stdint.h>`-shaped.

And the limit, stated because a previous tool in this tree got exactly this
wrong and read as a security finding: `agree` is a **lower bound**. arm64's
`atomic.h` writes `_ATOMIC_OP_IMPL(32, w, , op, ...)` and the name
`atomic_add_32` never appears in the file, so no regex finds it. The `gen`
column counts those invocations; where it is large — `atomic.h` at 197,
`cpu.h` at 175 — the agreement number is an underestimate and the header
deserves reading rather than scoring.

`pmap.h` and `cpu.h` sit at the bottom with small gaps, which is the tool
agreeing with the earlier conclusion: page table layout and CPU control
registers are not one interface written several times, they are several
interfaces.

## How many implementations, not how many copies

`tools/arch_clusters.py`. Single-linkage clustering over the similarity
graph, reporting "n copies, k implementations" and the lines that one
implementation per cluster would remove.

```
 saves  lines  n  k  file                     clusters
  9032  27097  3  2  linux_systrace_args.c    amd64,arm64 | i386
   614   2458  4  3  busdma_machdep.c         arm64,riscv | arm | powerpc
   469    938  6  3  uio_machdep.c            arm,i386,powerpc | arm64,riscv | amd64
   338   1014  3  2  vmm_dev.h                arm64,riscv | amd64
   289    867  3  2  acpi_machdep.c           amd64,i386 | arm64
   216    866  4  3  _inttypes.h              arm64,riscv | arm | powerpc
   198   1191  6  5  mem.c                    arm64,riscv | amd64 | arm | i386 | powerpc
   191    765  4  3  _stdint.h                arm64,riscv | arm | powerpc
     0  37831  5  5  pmap.c                   all five separate

over the whole list: 14,119 lines in 49 files
```

Three things fall out of it that the pairwise measure hid:

* **`uio_machdep.c` is not one implementation.** `arch_duplication.py` ranks
  it second in all of `sys/` at 0.97 and `docs/PORTABILITY.md` called it "the
  clearest case" for hoisting. It is three texts and two mechanisms:
  `pmap_map_io_transient` on amd64, arm64 and riscv, `sf_buf_alloc` on arm,
  i386 and powerpc — a direct map or the absence of one. Hoisting to one file
  means breaking three architectures.
* **`pmap.c` saves nothing.** Five copies, five implementations, 37,831
  lines, and the tool ranks it last. That is the same answer
  `arch_interface.py` gives from the other direction, and both agree with
  the reasoning that was already written down: page table layout is fixed by
  the MMU.
* **`_stdint.h` and `_inttypes.h` cluster into three**, and the generic
  headers already written collapse all three into one — because they work
  from the compiler's model of the target rather than from the text, so the
  clustering does not bound what is possible there. It bounds what
  *text-shaped* consolidation can do, which is the point of having both
  tools.

The threshold is single linkage on purpose: a chain of similar pairs counts
as one implementation, so `k` is a lower bound and the tool understates how
many distinct things there are. Understating is the safe direction — it
claims less consolidation is available, not more.
