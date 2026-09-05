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

That unevenness is a live bug source, not just untidiness: `atomic_add_8`
exists on amd64 and i386 and on no other architecture, so kernel code using it
compiles on x86 and fails to build on arm64 — which is exactly the kind of
breakage a tree that only ever built amd64 would not have noticed. The
remaining 13 names are i386 legacy variants (`cmpset_i386`, `store_i586`).

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
| six `atomic.h` | 5,107 lines | `std::atomic` / `__atomic` builtins |
| eliminable `.S` | 28,820 lines | generic C already present, via `MK_MACHDEP_OPTIMIZATIONS` |
| `_stdint.h`, `_inttypes.h` | 960 lines, 4-5 copies | `<cstdint>` is literally this |
| `stand/ficl/<arch>/sysdep.{c,h}` | 810 duplicated lines, 6 arches, 0.95 similar | one implementation |
| `lib/libc/<arch>/softfloat/softfloat.h` | 333 duplicated lines | one `basic_float<E,M>` template |
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
