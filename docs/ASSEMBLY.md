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
| — irreducible | 95 | 20,339 |
| — eliminable | 151 | 18,113 |
| — test-only | 7 | 1,394 |
| — not assembly (linker scripts) | 4 | 310 |

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
- **String, memory and math routines** — 144 of the 151 eliminable files.
  These exist for speed, not necessity; C fallbacks already exist and modern
  clang matches most of them. Measure before assuming a rewrite is a loss.
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

40,156 lines down to **20,339**, of which 1,704 more is test fixtures and
linker scripts that were never system code. Call it 18,113 lines of real
removable assembly, and a floor of 20,339 that stays.

Not zero — no kernel that boots on real hardware reaches zero, and the ones
that claim to are counting `__asm__` as not-assembly.

Ratchet it the way `docs/migration/freebsd_verified_floor.txt` ratchets
verified ports: a number that may only go down, checked in CI.
