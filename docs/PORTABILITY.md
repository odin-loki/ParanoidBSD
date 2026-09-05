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

## What the six-architecture matrix actually found

Building all six is the measurement. Run 7 of `pbsd-arch-matrix` was the first
where every job got past the reporters and into a compiler, and it found four
bugs. None of them is visible from amd64, and all four are still in
HardenedBSD 15-stable — three of the four files are byte-identical to a fresh
upstream clone, and the fourth differs only where upstream has moved on.

| arch    | run 7 | run 8 | run 9 | what stopped it                          |
|---------|-------|-------|-------|------------------------------------------|
| amd64   | pass  | pass  | pass  |                                          |
| riscv   | pass  | pass  | pass  |                                          |
| arm64   | fail  | pass  | pass  | `sun50i_a64_acodec.c`, a removed accessor |
| powerpc | fail  | pass  | pass  | `phyp_vscsi.c`, a missing semicolon       |
| i386    | fail  | fail  | pass  | a removed source, then a missing variable |
| arm     | fail  | fail  | pass  | 32-bit ASLR, then the ifunc shims         |

**Run 9 is green on all six.** That is the first time every architecture
PBSD claims as first tier has built the HardenedBSD kernel in one run. Six
bugs, none of them visible from amd64, all of them still in HardenedBSD
15-stable:

**arm — `sys/hardenedbsd/hbsd_pax_aslr.c`.** The `#else /* ! __LP64__ */`
branch never got a `PAX_ASLR_DELTA_THR_STACK_DEF_LEN`, so thread-stack ASLR
does not compile on any 32-bit architecture. Separately, the whole MAP_32BIT
half of the file is keyed off `#ifdef MAP_32BIT` — but `<sys/mman.h>` defines
that flag on every architecture, while `struct vmspace` only carries
`vm_aslr_delta_map32bit` under `__LP64__`, and `vm_map.c` guards its two uses
the same way. Three files disagreed about one condition. They now all say
`__LP64__`, and a MAP_32BIT mapping on a 32-bit kernel is randomised as an
ordinary mmap instead of out of a delta that does not exist there.

i386 has the same bug and did not report it: under `-j`, the module tree
failed first.

**arm64 — `sys/arm/allwinner/a64/sun50i_a64_acodec.c`.** PBSD carries a newer
sound stack than the HardenedBSD clone; that version publishes `struct
snd_mixer` in `mixer.h` and drops `mixer_get_lock()`. One driver still called
it, and C's implicit-declaration rule turned that into `struct mtx *` from
`int`. Same shape as the `hwpstate_intel.c` bug in run 6: a newer shared
subsystem, an older consumer of it, and only a non-amd64 build compiles the
consumer.

**i386 — `sys/modules/linux/Makefile`.** Lists `imgact_linux.c` for i386.
The file is not in this tree and not in upstream's; the Linux a.out image
activator was removed years ago. Nobody builds the i386 linux module.

**powerpc — `sys/powerpc/pseries/phyp_vscsi.c:341`.** `return (ENOMEM)` with
no semicolon. A syntax error, upstream, in a file that is in the powerpc
kernel config — which means nothing in HardenedBSD's CI compiles it.

**arm — `sys/kern/sched_shim.c`.** FreeBSD 15's pluggable scheduler defines
its 43 entry points as ifuncs, and `sys/arm/include/ifunc.h` is nine lines
that say `__DO_NOT_HAVE_SYS_IFUNCS` and define no `DEFINE_IFUNC`. The file
is `standard` in `sys/conf/files`, so arm could not build it at all.
`libkern/gsb_crc32.c`, the only other machine-independent user, already
carries a plain-C `#else`; this is that branch for the scheduler. One load
of `active_sched` per call instead of a resolve-once branch — the trade PBSD
exists to make.

**i386 — `i386_read_exec`.** Declared in `sys/x86/include/x86_var.h`, read
five times in i386's `pmap.c`, defined nowhere in either tree. The
definition went with the PAX_NOEXEC rework of `vm_mmap.c` and the uses did
not; `vm_mmap.c` still carries the comment `/* for i386_read_exec */` on an
include kept for a variable that is not there. Defined as 0 with no sysctl:
in stock FreeBSD it is a knob for letting readable segments be executable,
which on a PAX_NOEXEC system is a switch for turning off the point of it.

Each of these is a one-line fix and none was findable by reading.
`tools/check_pbsd_marks.py` guards all of them by content, because an
upstream merge takes upstream's side on a vendor file without a conflict.

The rate is worth noting: six runs, six bugs, one per run, roughly an hour
apart — because each architecture stopped at its first error and hid the
next. The matrix now runs with `make -k` so a survey reports the whole list
in one pass; the release path still stops at the first failure.

## SafeStack and CFI: the runtime, not the flag

`WITH_SAFESTACK` and `WITH_CFI` are in `hbsd/src.conf.pbsd`, and the question
of whether they work per architecture is not about the compiler flag. Both
make clang link a compiler-rt archive, and `lib/libclang_rt/Makefile` says
which architectures the tree builds one for:

| arch    | SafeStack rt | CFI rt | ASan | UBSan |
|---------|--------------|--------|------|-------|
| amd64   | yes          | yes    | yes  | yes   |
| arm64   | yes          | yes    | yes  | yes   |
| i386    | yes          | yes    | yes  | yes   |
| arm     | no           | no     | no   | no    |
| powerpc | no (32-bit)  | no     | 64-bit only | 64-bit only |
| riscv   | no           | no     | yes  | yes   |

So on arm, powerpc and riscv there is no SafeStack or CFI to have, whatever
`src.conf` says. `src.opts.mk` defaults both off outside amd64 and aarch64 —
and `hbsd/src.conf.pbsd` was overriding that default on every architecture,
on the argument that upstream's limit was about test coverage rather than
hardware.

Matrix run 8 is where that showed. `show_hardening.sh` reported, faithfully,
for `arm/armv7`:

```
  SAFESTACK          yes   (expected where the architecture supports it)
  CFI                yes   (expected where the architecture supports it)
```

Both are enabled, and neither has a runtime on arm. Nothing failed, because
the matrix builds kernels and a kernel does not link `libclang_rt`. A world
build would have failed at the first PIE binary, the way run 8 and run 9 of
`pbsd-boot-image` did on amd64 for the toolchain's version of the same
problem.

`src.conf.pbsd` now conditions those two on `MACHINE_CPUARCH` being one of
amd64, aarch64 or i386. That is not a retreat from first-class support: for
`RETPOLINE` and `BRANCH_PROTECTION` the "upstream is being careful, the
hardware is fine" argument holds and they are unchanged. For these two the
blocker is that the runtime does not exist, which is a fact about the tree
rather than a judgement about it. Building it for arm, powerpc and riscv is
the way to close the gap, and it is open work rather than a configuration
line.

The external toolchain adds a second condition. `WITHOUT_TOOLCHAIN` sets
`MK_CLANG=no`, which stops `lib/libclang_rt` being built at all, so the
archive has to come from the packaged clang — and the FreeBSD `llvm21` package
ships none. Run 9 found that as `ld.lld: error: cannot open
libclang_rt.safestack.a` while linking `bin/cat`, sixteen minutes into
`buildworld`. `tools/ci/build_boot_image.sh` now asks the packaged clang for
its resource directory before starting, prints every `libclang_rt.*` it has,
and writes `WITHOUT_SAFESTACK` / `WITHOUT_CFI` into the generated `src.conf`
when the archive is absent. `tools/ci/show_hardening.sh` then reports what the
build settled on, so the option being off is a line in the log rather than a
link error later.

## The boot test has not run yet

`tools/ci/boot_test.py` exists and `pbsd-boot-image.yml` has a "Boot it"
step, and it has been **skipped in every run so far** — runs 8, 9 and 10 all
failed in "Build on FreeBSD" before an image existed, so `Collect image`,
`Boot it` and the artifact upload were all skipped.

Runs 8 and 9 failed for the same reason, sixteen minutes in each time:

```
ld.lld: error: cannot open .../libclang_rt.safestack.a: No such file or directory
```

Run 8 was `memstick` on `80afbae63` and run 9 was `world` on `b1bd99b89`, so
the untracking of `contrib/llvm-project` is not what broke run 9 — run 8 hit
the identical error on the tree before it. Both are the SafeStack runtime,
and the probe added to `build_boot_image.sh` is what stops a third run
finding it a third time.

So there is still no answer to which of `boot_test.py`'s four outcomes PBSD
hits. It is the next thing a green world run buys.
