# Assembly, and why "all ISO C++23" has a floor

**The point of removing assembly is portability, not speed.** Every line of
architecture-specific assembly is a line that a new architecture has to
provide before it can boot, and most of it is providing something that
already exists in architecture-neutral C. Losing some performance to gain a
port is the trade this is making deliberately.

The goal is right, and it is worth being precise about where it stops,
because the stopping point is not a matter of effort.

**ISO C++23 has no construct that emits these instructions.** The standard
deliberately says nothing about registers, privilege levels, or what runs
before `main`. This is not a gap in the language; it is the language
declining to describe a machine.

Note that `__asm__` and compiler intrinsics (`_mm_aesenc_si128`,
`__builtin_ia32_*`) are **not ISO C++ either**. "No `.S` files, but inline
asm" is a different and much weaker claim than the one worth making.

## The floor

`python3 tools/asm_inventory.py`, measured 2026-09-05:

| | files | lines |
|---|---:|---:|
| vendored third-party | 533 | 624,548 |
| **PBSD's own** | **427** | **71,905** |
| — irreducible | 201 | 40,788 |
| — eliminable | 215 | 29,413 |
| — test-only | 7 | 1,394 |
| — not assembly (linker scripts) | 4 | 310 |

These numbers are for the whole tree, with `arm`, `arm64`, `amd64`, `i386`,
`powerpc` and `riscv` all present. An earlier revision of this page measured
40,156 lines because the snapshot had dropped four of those architectures.
Restoring them is the point: a tree that only builds amd64 has not made the
assembly problem smaller, it has hidden most of it.

## What a new architecture costs

`python3 tools/asm_inventory.py --by-arch`:

| architecture | files | lines | irreducible | eliminable |
|---|---:|---:|---:|---:|
| amd64 | 89 | 16,484 | 7,749 | **8,581** |
| i386 | 72 | 13,733 | 7,272 | **6,379** |
| arm | 57 | 12,410 | 6,361 | **6,049** |
| powerpc | 58 | 12,109 | 10,864 | 1,245 |
| arm64 | 58 | 10,384 | 5,195 | **4,076** |
| riscv | 28 | 3,932 | 2,921 | 1,011 |
| arch-neutral | 65 | 2,853 | 426 | 2,072 |

This is the whole argument in one table. On amd64, arm and i386 there is
**more eliminable assembly than irreducible** — more than half of what those
ports carry is hand-written versions of routines that already have
architecture-neutral C. Every new architecture pays that cost again from
scratch.

riscv already looks like the target shape: 2,921 lines of genuine nucleus
against 1,011 of optional. Bringing the others to that ratio is what makes
the next port cheap, and it is worth roughly 29,000 lines.

Nothing is unclassified. The 77 that were have been sorted by hand, and the
rules that sort them run only on what the earlier rules leave over, so
adding one cannot silently change a verdict already reached.

94% of the assembly by line count is LLVM's test suite, `netbsd-tests` and
`arm-optimized-routines` — upstream's code, not PBSD's to rewrite. Counting
it buries the number that matters.

On amd64 the irreducible core is ten files:

| File | Why C++23 cannot express it |
|---|---|
| `locore.S`, `mpboot.S` | The first instructions after the loader. No stack exists yet — `%rsp` must be set before any compiled code can run. |
| `exception.S`, `apic_vector.S`, `atpic_vector.S` | Hardware vectors to a fixed address with its own register state. All registers must be saved before compiler-generated code touches them, and the return is `IRET`. |
| `cpu_switch.S` | Saves and restores a named register set and swaps stacks. C++ has no notion of "the register file". |
| `support.S`, `efirt_support.S` | `CR0/CR3/CR4`, `LGDT/LIDT/LTR`, `WRMSR/RDMSR`, `INVLPG`, `CPUID`, `HLT`, `STI/CLI`. |
| `sigtramp.S` | Trampoline bytes planted on the user stack; must be exact. |
| `xen-locore.S` | Xen PV entry, same class as `locore.S`. |

## What does go

- **Atomics and barriers** → `std::atomic`, `std::atomic_ref`,
  `std::atomic_thread_fence`.
- **Bit twiddling** → `<bit>`: `popcount`, `countl_zero`, `countr_zero`,
  `byteswap`, `bit_cast`. Replaces hand-written BSF/BSR/POPCNT/BSWAP.
- **String, memory and math routines** — 197 of the 215 eliminable files.
  Mostly not rewrites at all: `lib/libc/amd64/string/strcat.S` sits beside
  `lib/libc/string/strcat.c`, and removing it means deleting the `.S` and
  letting the generic C build. A Makefile edit, not a port.
- **ARM EABI helpers** — `aeabi_vfp_double.S` and friends implement
  soft-float and division for targets without VFP. compiler-rt ships C for
  every one of them.
- **ELF notes** — `crtbrand.S`, `feature_note.S`, `ignore_init_note.S` emit
  `.note` sections and nothing else. A struct with a section attribute does
  the same job, though the attribute is not ISO C++ either.

## What the sorting turned up

The 77 files that were unclassified were not a long tail of easy cases.
Sorting them moved the irreducible floor from 17,070 lines to 20,339, and
most of the movement is one idea repeated: **the file exists because the
instruction is the point.**

- `lib/csu/*/crt1_s.S`, `crti.S`, `crtn.S` — what runs before `main`.
- `lib/libsys/*/cerror.S` — amd64 signals syscall failure in the carry flag.
  C++ cannot see flags.
- `vfork.S`, `rfork_thread.S` — `vfork` must not touch the stack between the
  syscall and the return; the `rfork` variants return onto a different stack.
- `copyinout.S`, `copyout_fast.S` — the fault fixup table is keyed on the
  address of the faulting instruction, so the instruction must be named.
- `firmw.S`, `embedfs.S`, `fdt_static_dtb.S` — `.incbin`. C++26 gets
  `#embed`; C++23 does not have it.
- `subr_ticks.S` — `ticks = ticksl + offset`, a symbol defined at an offset
  from another symbol.
- `acpi_wakecode.S`, `bioscall.S`, `vm86bios.S` — real mode.
- `smccc_*.S`, `hyp_stub.S`, `vmm_call.S` — `SMC`, `HVC`, exception-level
  transitions.

Two categories came out of it that are not "assembly to remove" at all:
four `.lds.s` files are **linker scripts** run through cpp, and seven files
under `tests/`, `tools/regression/` and `tools/test/` are **assembly under
test** — rewriting them in C++ would delete the thing being tested.

## The target

71,905 lines down to **40,788**, of which 1,704 more is test fixtures and
linker scripts that were never system code. 29,413 lines of real removable
assembly, and a floor of 40,788 that stays.

Measured per architecture, that is the difference between a new port needing
roughly 12,000 lines of assembly and needing roughly 6,000 — and the 6,000
that remains is the part that genuinely cannot be written any other way.

Not zero — no kernel that boots on real hardware reaches zero, and the ones
that claim to are counting `__asm__` as not-assembly.

Ratchet it the way `docs/migration/freebsd_verified_floor.txt` ratchets
verified ports: a number that may only go down, checked in CI.

## The option was not doing anything for lib/msun on x86

`WITHOUT_MACHDEP_OPTIMIZATIONS` is the switch this whole document rests on:
one line in `hbsd/src.conf.pbsd` that selects the machine-independent C over
the hand-written assembly, in `lib/libc`, `lib/msun` and `lib/libmd`. Two of
those three implement it correctly. `lib/msun` did not.

```make
.if defined(ARCH_SRCS) && "${MK_MACHDEP_OPTIMIZATIONS}" != no
.for i in ${ARCH_SRCS}
COMMON_SRCS:=  ${COMMON_SRCS:N${i:R}.c}
.endfor
.endif

SRCS=	${COMMON_SRCS} ${ARCH_SRCS}
```

With the option **on**, the loop drops `e_fmod.c` and `e_fmod.S` is built —
the intent. With it **off**, the loop is skipped and nothing drops the
assembly, so `SRCS` holds both `e_fmod.c` and `e_fmod.S`. Those are the same
object name. Which one bmake builds is decided by suffix-rule precedence and
`.PATH` order, not by the option.

Measured rather than reasoned about:

| arch | `ARCH_SRCS` | collide with a `COMMON_SRCS` `.c` |
|---|---:|---:|
| amd64 | 23 | **23** |
| i387 (i386) | 47 | **47** |
| arm | 2 | 0 |
| aarch64, powerpc, riscv | 0 | 0 |

Every single machine-dependent source on x86 collides. arm's two are
`fenv-softfp.c` and `fenv-vfp.c`, which have no machine-independent
counterpart and are genuinely additional — which is why the fix is a filter
and not `ARCH_SRCS=`.

The other two libraries get it right, and comparing them is how the shape of
the bug is clearest:

* `lib/libc/Makefile:159` — `.if empty(MDSRCS) || ${MK_MACHDEP_OPTIMIZATIONS}
  == no` then `SRCS+= ${MISRCS}`, and `MDSRCS` is appended only in the
  `.else`. One or the other, never both.
* `lib/libmd/Makefile:114` — `USE_ASM_SOURCES:=0` and the assembly is never
  reached.
* `lib/msun/Makefile:193` — appends both and relies on a loop that only runs
  in the other configuration.

Fixed by dropping, when the option is off, exactly those `ARCH_SRCS` entries
that have a `COMMON_SRCS` counterpart. `tools/check_pbsd_marks.py` holds the
fix, because it is an edit to a vendor file that an upstream merge would take
back without a conflict.

**What this means for the numbers above.** Everything in this document about
`lib/libc` stands — that library implements the option correctly, and the 74
`.S` files deleted from it were deleted, not switched off. The `lib/msun`
figures were describing an option that was not taking effect on the two x86
architectures. The next `buildworld` is what says how much moves.

## The removal was five entries and six files short

`tools/check_source_includes.py` and `tools/check_libc_srcs.py`, both
written for other reasons, together found what the removal of the 74 `.S`
files left behind. Three loose ends, none of them visible in any build PBSD
runs:

* `lib/libc/amd64/stdlib/Makefile.inc` was one line, `MDSRCS+=div.S ldiv.S
  lldiv.S`, and all three files were deleted. `lib/libc/i386/stdlib` the
  same for two. The commit said the `MDSRCS` entries were "dropped from six
  Makefile.inc files" — the `string` ones were, the `stdlib` ones were not.
* `lib/libc/powerpc64/string` kept `bcopy_vsx.S`, `memcpy_vsx.S` and
  `memmove_vsx.S` in `MDSRCS` after their templates went. Each is macro
  definitions ending in `#include "bcopy.S"`, so none of them can compile.
  The three `_resolver.c` beside them are just as dead: `bcopy_resolver.c`'s
  ifunc chooses between `__bcopy_vsx` and the `__bcopy` that `bcopy.S`
  defined.

All six are now removed with their entries, and `strcpy`/`strncpy` keep
their POWER ISA 2.05 variants, which are complete on disk.

**Why nothing caught it.** `MK_MACHDEP_OPTIMIZATIONS=no` drops `MDSRCS`
whole (`lib/libc/Makefile:159`), and that is what `src.conf.pbsd` asks for,
so an `MDSRCS` entry naming a file that does not exist is inert. It stays
inert right up until something asks for the assembly back — and
`pbsd-boot-image`'s `machdep_asm` input does exactly that. `powerpc` in the
matrix is `TARGET_ARCH=powerpc64`, so this was one of the six.

`check_libc_srcs.py` resolves the way bmake does: lib/libc is one build with
one `.PATH` list, and a source named anywhere can live in any directory on
it. Resolving per-directory instead reported nineteen sources that are all
perfectly findable — `ldexp.c` in `gen`, `machdep_ldis*.c` in `gdtoa`,
`softfloat.c` under `softfloat/bits{32,64}` — and a check that cannot tell
those from the five real ones is not a check.
