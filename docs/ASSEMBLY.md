# Assembly, and why "all ISO C++23" has a floor

The goal of removing assembly is right, and it is worth being precise about
where it stops, because the stopping point is not a matter of effort.

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
| **PBSD's own** | **257** | **40,156** |
| — irreducible | 63 | 17,070 |
| — eliminable | 117 | 12,150 |
| — unclassified | 77 | 10,936 |

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
- **String, memory and math routines** — 113 of the 117 eliminable files.
  These exist for speed, not necessity; C fallbacks already exist and modern
  clang matches most of them. Measure before assuming a rewrite is a loss.

## The target

40,156 lines down to roughly 17,000. Not zero — no kernel that boots on real
hardware reaches zero, and the ones that claim to are counting `__asm__` as
not-assembly. The 77 unclassified files are the next thing to sort; the
classifier is deliberately conservative and puts anything it cannot place
there rather than guessing.

Ratchet it the way `docs/migration/freebsd_verified_floor.txt` ratchets
verified ports: a number that may only go down, checked in CI.
