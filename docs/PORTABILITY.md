# Portability: what a new architecture costs, and why

Removing assembly is for portability. Every hand-written `.S` and every
per-architecture copy of shared logic is something a new port must provide
before it boots.

## The three measurements

```sh
python3 tools/asm_inventory.py --by-arch    # assembly, per architecture
python3 tools/arch_contract.py hbsd/src     # what an architecture must provide
python3 tools/arch_duplication.py hbsd/src  # what several architectures each re-implement
```

### Assembly

| architecture | files | lines | irreducible | eliminable |
|---|---:|---:|---:|---:|
| amd64 | 89 | 16,484 | 7,749 | **8,581** |
| i386 | 72 | 13,733 | 7,272 | **6,379** |
| arm | 57 | 12,410 | 6,361 | **6,049** |
| powerpc | 58 | 12,109 | 10,864 | 1,245 |
| arm64 | 58 | 10,384 | 5,195 | 4,076 |
| riscv | 28 | 3,932 | 2,921 | 1,011 |

On amd64, arm and i386 there is more eliminable assembly than irreducible.
`WITHOUT_MACHDEP_OPTIMIZATIONS` in `src.conf.pbsd` removes 12,344 lines of it
in libc alone, using `lib/libc/Makefile:159`, upstream's own switch.

### The contract

61 headers and 15 sources that every one of the six architectures provides.
Derived by intersecting them, not written by hand, so it re-derives when an
architecture is added. It is what found that `arm` and `powerpc` had no
HardenedBSD kernel config.

### The duplication

`lib/libc` has the right shape — machine-dependent wins, machine-independent
is the automatic fallback, and the whole MD layer can be switched off. `sys/`
has no equivalent: what is under `sys/<arch>` is unconditional, so where two
architectures need the same logic they each keep a copy, and the copies drift.

That is not hypothetical. The HardenedBSD hardening policy was six copies and
riscv's was missing `HBSD_RESIST_FINGERPRINTING`.

`arch_duplication.py` ranks the same shape in code. The top of the list:

| similarity | file | architectures | identical lines |
|---:|---|---|---:|
| 0.97 | `uio_machdep.c` | all six | 55 common to all |
| 0.95 | `mem.c` | all six | — |
| 0.99 | `_inttypes.h` | arm, arm64, powerpc, riscv | 111 |
| 0.95 | `_stdint.h` | arm, arm64, powerpc, riscv | 56 |
| 0.96 | `float.h` | arm, arm64, powerpc, riscv | 35 |
| 0.91 | `busdma_machdep.c` | arm, arm64, powerpc, riscv | 23 |

4,679 lines are identical across every architecture that has the file,
summed over the 26 candidates that score above 0.5.

`sys/x86` already exists and is exactly this move, made once for amd64 and
i386. `uio_machdep.c` at 0.97 across all six is the clearest case for making
it again.

**Nothing has been hoisted yet.** Moving a file out of `sys/<arch>` means
editing six `sys/conf/files.<arch>`, and until `buildworld` is green a failure
afterwards cannot be attributed to the move. The measurement comes first; it
is the part that was missing.

## What is generated, and does not count

`linux_proto.h`, `linux_sysent.c` and `linux_systrace_args.c` score 0.84-0.86
across three architectures with over a thousand identical lines each. They are
generated from `syscalls.master`. The duplication is in the output, not in
anything a person maintains, and hoisting them would be working on the wrong
end.
