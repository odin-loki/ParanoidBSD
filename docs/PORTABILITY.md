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
i386.

**`uio_machdep.c` at 0.97 is not the clearest case for making it again**,
which is what this section used to say. 0.97 is the *best pair*, and reading
the six files rather than the score gives a different answer:

```
pmap_map_io_transient   amd64, arm64, riscv     architectures with a direct map
sf_buf_alloc            arm, i386, powerpc      architectures without one
```

Two implementations, and the split is a real hardware property. Hoisting to
one file means picking one of them and breaking three architectures. The
correct change is two machine-independent implementations and a way to
choose, which is a different and larger piece of work than the score
suggested.

A pairwise maximum cannot show that, so `tools/arch_clusters.py` clusters
instead — single linkage over the similarity graph — and reports "n copies,
k implementations". It is the tool that would have stopped the sentence
above being written. `pmap.c` comes out 5 copies and 5 implementations,
saving nothing, which agrees with `arch_interface.py` and with the reasoning
already in this document.

**Nothing has been hoisted yet.** Moving a file out of `sys/<arch>` means
editing six `sys/conf/files.<arch>`, and `uiomove_fromphys` is a hot VM path
where a mistake is data corruption rather than a compile error. The six-arch
matrix is green now, so a build failure after such a change would be
attributable — but the matrix proves it compiles, not that it runs, and
PBSD's kernel mounts root and goes no further (run 20; run 21 is the one
that shows it reaching the exec). That is the gate on this one.

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

The external toolchain adds a second condition, and it turned out to be a
different one from what it looked like. `WITHOUT_TOOLCHAIN` sets
`MK_CLANG=no`, which stops `lib/libclang_rt` being built, so the archive has
to come from the packaged clang. Runs 8 and 9 both died sixteen minutes into
`buildworld` on

```
ld.lld: error: cannot open
  /usr/local/llvm21/lib/clang/21/lib/x86_64-unknown-freebsd15.1/libclang_rt.safestack.a
```

The obvious reading — the package does not ship a SafeStack runtime — is
wrong, and the probe added after run 9 was built on it. Run 11 printed what
the package actually has:

```
   x86_64-portbld-freebsd15.0/libclang_rt.safestack.a
   x86_64-portbld-freebsd15.0/libclang_rt.cfi.a
```

Both archives are there. They are under the port's own default triple, and
`Makefile.inc1:146` builds the world with `-target
x86_64-unknown-freebsd15.1`, so clang looks in a directory that does not
exist. Vendor and FreeBSD minor version differ; the archives do not.

A probe that asked "does a `libclang_rt.safestack` exist anywhere under the
resource directory" answered yes, left the option on, and run 11 failed at
`bin/cat` exactly as run 9 had. **That is the fifth time in this work that a
query returned something which parsed as an answer to a question it was not
asking**, and the pattern is the same every time: the check was written from
the expected failure rather than from the actual interface.

`build_boot_image.sh` now derives the build's triple the way
`Makefile.inc1` does — `TARGET_ARCH` with amd64 spelled `x86_64`, the ABI,
and `OS_REVISION` out of `sys/conf/newvers.sh` — and asks `clang -target
<that> -print-runtime-dir`. If the archives are under one other FreeBSD
triple it symlinks the directory the build asks for onto the one the port
built, says so, and keeps the mitigation; if there is nothing to alias it
writes `WITHOUT_SAFESTACK` / `WITHOUT_CFI` into the generated `src.conf` and
`show_hardening.sh` reports the option as off. Tested against four layouts —
the port's triple, the tree's triple, an empty resource directory, and a
directory with other runtimes but not these — before being run again.

**Run 12 says it works.** The probe reported

```
build target=x86_64-unknown-freebsd15.1
runtime-dir=.../lib/clang/21/lib/x86_64-unknown-freebsd15.1
what the package actually has:
  x86_64-portbld-freebsd15.0
the package built its runtime for x86_64-portbld-freebsd15.0
and this build asks for x86_64-unknown-freebsd15.1; aliasing
SAFESTACK on: .../x86_64-unknown-freebsd15.1/libclang_rt.safestack.a
CFI on:       .../x86_64-unknown-freebsd15.1/libclang_rt.cfi.a
```

and then

```
>>> World build started   on Sat Sep  5 07:58:11 UTC 2026
>>> World build completed on Sat Sep  5 08:25:07 UTC 2026
```

Twenty-seven minutes, zero errors, **with SafeStack and CFI on**, on the
tree with `contrib/llvm-project`'s compiler untracked. Runs 8, 9 and 11 each
died at sixteen minutes on the missing archive; this is the first world
build to finish since run 7, and the first ever to finish with those two
mitigations actually linking.

## The boot test, and what it has and has not established

`tools/ci/boot_test.py` exists and `pbsd-boot-image.yml` has a "Boot it"
step. It was **skipped in every run up to 13** — runs 8, 9 and 10 all failed
in "Build on FreeBSD" before an image existed, so `Collect image`, `Boot it`
and the artifact upload were all skipped.

Runs 8 and 9 failed for the same reason, sixteen minutes in each time:

```
ld.lld: error: cannot open .../libclang_rt.safestack.a: No such file or directory
```

Run 8 was `memstick` on `80afbae63` and run 9 was `world` on `b1bd99b89`, so
the untracking of `contrib/llvm-project` is not what broke run 9 — run 8 hit
the identical error on the tree before it. Both are the SafeStack runtime,
and the probe added to `build_boot_image.sh` is what stops a third run
finding it a third time.

### It has run now, and the kernel boots

| run | stage | how far | what stopped it |
|---|---|---|---|
| 14 | `vm` | no image | `vm-image` is a no-op stamp without `WITH_VMIMAGES`, and the name is `vm.ufs.raw`, not `vm.raw` |
| 15 | `memstick` | loader | reported `OK booted` in four seconds by matching the boot **menu title** — a false positive, since corrected |
| 16 | `memstick` | loader | the console command was truncated at fifteen characters by a UART FIFO overrun; `boot` was never sent |
| 19 | `memstick` | build | the port-built gate tripped on vendor lldb C++; killed before the boot test |
| 20 | `memstick` | **kernel** | boots, mounts root, then silence at `exec /sbin/init` |
| 21 | `memstick` | **kernel** | `start_init: trying /sbin/init` printed, then silence: the exec is reached |
| 22 | `memstick` | **kernel** | `init_path=/rescue/sh` did not take effect — still `/sbin/init`, so the discriminator never ran |
| 23 | `memstick` | **kernel** | `init_path=/rescue/sh:/sbin/init` confirmed received by the loader; three console pokes, no reply. Whether the kernel used it is unobserved — `boot_verbose` was off |
| 25 | `memstick` | **kernel** | all three loader commands confirmed by echo, and the kernel still ran `/sbin/init`. A loader-prompt `set` does not reach `kern_getenv()` |
| 26 | `memstick` | build | the loader.conf injection met a real image and found it is MBR with a BSD label, not GPT. No boot |
| 17 | `vm` | staging | `installworld` into the image mount stopped dead at `usr.sbin/inetd`; 2h23m with no further output, cancelled |
| 24 | `vm` | staging | the same hang, at `usr.sbin/lpr/lpc`; 1h36m, cancelled. Reproducible |

### The `vm` stage hangs reproducibly, and is not on the critical path

`vm-image` with `WITH_VMIMAGES=YES VMFORMATS=raw VMFSLIST=ufs` does real
work — it attaches an image and installs the world into
`release/vm-image-raw-ufs/` — and has now hung twice at the same stage:

| run | last line before silence | silent for |
|---|---|---|
| 17 | `usr.sbin/inetd` man pages | 2h23m, cancelled |
| 24 | `usr.sbin/lpr/lpc` `_debuginstall`, then `usr.sbin/mlx5tool` | 1h36m, cancelled |

Both inside `realinstall_subdir_usr.sbin`, writing many small files into
the mounted image. Twice at the same stage is reproducible rather than
bad luck, and neither run produced `vm.ufs.raw`.

**The cause is unknown and is not guessed at here.** Not re-dispatched a
third time: the `memstick` path builds and boots, `vm` is the convenience
of having a login prompt rather than a requirement, and a third fifty-
minute run that hangs in the same place buys nothing. It is worth
returning to when the boot question is settled — the `--run`
interrogation in `boot_test.py` needs a login, and `vm` is where that
lives.

Runs 14 to 19 all stopped in the harness rather than in the tree. Run 20
did not:

```
FreeBSD 15.1-STABLE-HBSD  HARDENEDBSD amd64
clang version 21.1.8
HardenedBSD: initialize and check features
             (__HardenedBSD_version 1500001 __FreeBSD_version 1501501).
vtblk0: <VirtIO Block Adapter> on virtio_pci0
uart0: console (115200,n,8,1)
Trying to mount root from ufs:/dev/ufs/HardenedBSD_Install [ro,noatime]...
Dual Console: Serial Primary, Video Secondary
```

**The kernel starts, initialises HardenedBSD's feature checks, probes the
disk, and mounts root.** Then nothing for six minutes fifty-one seconds —
no panic, no trap, no `mountroot>`.

The stopping point is exact rather than inferred. That last line comes
from `start_init()` at `sys/kern/init_main.c:765`, which sits *after*
`vfs_mountroot()` returns and *before* the loop that execs `/sbin/init`.
So root is mounted and the first userland process is what produces
nothing.

Run 21, the same image under `boot_verbose=YES boot_single=YES`, moves the
boundary one step further in:

```
start_init: trying /sbin/init
<420s of nothing>
```

The kernel **reaches the exec**. And the absences pin it: no `exec
/sbin/init: error N` (printed for any error but `ENOENT`), no second
`start_init: trying` (which `ENOENT` would cause), no `panic: no init`
(what running out of paths does), and no single-user shell prompt despite
`boot_single`.

So `kern_execve()` neither failed nor produced a program that spoke.
Whether it hangs, or `init` runs and hangs before its first write, is
still open.

### Runs 22 to 27: four ways of setting `init_path`, none of which worked

The obvious next test — point `init_path` at the statically linked
`/rescue/sh`, which replaces both `init` and the dynamic loader at once —
took four runs and never ran, because the setting never reached the
kernel.

| run | how `init_path` was set | what the kernel printed |
|---|---|---|
| 22 | `set` at the loader prompt, fixed sleep, no receipt | `trying /sbin/init` |
| 23 | `set` at the prompt, receipt required | (`boot_verbose` off) |
| 25 | `set` at the prompt, receipt + `boot_verbose` | `trying /sbin/init` |
| 27 | appended to `/boot/loader.conf` inside the image | `trying /sbin/init` |

Run 25 is the one that settles the prompt. All three commands were echoed
back in full and the kernel used the compiled-in default anyway, so a
receipt proves the loader *received* the line and nothing about
`kern_getenv()`.

Run 27 settles `loader.conf`. The injection worked — `gpart` found the
MBR slice, descended into it, mounted `md0s2a`, appended, and read the
file back showing `init_path` — and the kernel still said `/sbin/init`.

**The premise that route was chosen on was wrong**, and it was greppable.
The claim was that `loader.conf` reaches the kernel because the root
mount comes from `vfs.root.mountfrom` in it. `release/Makefile` writes
exactly three lines into that file and `vfs.root.mountfrom` is not one of
them; the loader sets that variable itself, in `stand/common/boot.c:384`,
from the device it loaded the kernel off. What the root mount shows is
that the loader's *own* environment reaches `kern_getenv()` — not that
`loader.conf` gets into that environment.

### What does reach the kernel, and by which road

`sys/kern/subr_boot.c` has a table, `howto_names`, of twelve variables —
`boot_single`, `boot_verbose`, `boot_serial`, `boot_multicons`,
`boot_askname` and the rest. `boot_env_to_howto()` turns them into
`boothowto` bits, and on this platform **the loader** calls it, in
`bi_getboothowto()`, before the kernel is entered. Those bits travel in
the bootinfo block. `init_path` has no such road: `start_init()` reads it
with `kern_getenv()`, out of the kernel environment the loader copies in
`md_copyenv()`.

Two roads, and only one of them is demonstrated. `boot_verbose` has
worked every run since 21 — `start_init: trying` prints under nothing
else — and `Dual Console: Serial Primary, Video Secondary` is
`RB_MULTIPLE | RB_SERIAL` arriving the same way. `init_path` has never
worked, by any of the four means above.

### What one `trying` line then silence actually means

Read the loop rather than the summary of it:

```c
error = kern_execve(td, &args, NULL, oldvmspace);
if (error == EJUSTRETURN) { ...; return; }      /* success */
if (error != ENOENT) printf("exec %s: error %d\n", path, error);
```

`ENOENT` prints the *next* `trying` line; any other error prints `exec
/sbin/init: error N`. Neither happened, so there are exactly two
readings: `kern_execve()` never returned, or it returned `EJUSTRETURN` —
**and init is running.** One `trying` line followed by nothing is what a
*successful* exec looks like from the kernel's side.

Against "init is running", run 21 is the evidence: it set `boot_single`
as well, and single-user init prints `Enter full pathname of shell or
RETURN for /bin/sh:` before it execs anything (`sbin/init/Makefile` builds
with `-DDEBUGSHELL -DSECURE`). No such line appeared.

That is evidence and not proof, and the difference is the same one this
section has been about four times. Run 21's `boot_single` was echoed back
by the loader; whether it reached `boothowto` is unobserved, exactly as
`init_path`'s receipts were. The inference is stronger here — `boot_verbose`
travelled the identical road in the same run and did take effect — but a
run that put `boot_single` in `loader.conf` instead of at the prompt would
be testing the one thing run 25 showed can differ.

See `docs/BUILDING.md` for the console mechanics and the fixes that made
these runs possible (type one byte at a time; require a receipt before
claiming the console was set; print the loader record at the end, where a
verbose boot cannot bury it).
