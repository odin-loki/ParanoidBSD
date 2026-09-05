# What is left, and what each thing costs

A working list, not a plan with dates. Everything here is either measured or
marked as an estimate, because the recurring failure in this work has not
been doing the wrong thing — it has been believing a number nobody checked.

## The rules this list is written under

Five of them, each paid for:

1. **A query that cannot answer still returns something.** `make -V`,
   `make -f Makefile.inc1 -V`, `showconfig` parsed with the wrong field
   separator, the atomics parity tool, and the `libclang_rt` probe — five
   checks that produced confident answers to questions they were not asking.
   Every one read as a finding. Test a check against a known input before
   shipping it.
2. **Score, then read.** Text similarity ranked `atomic.h` last and
   `uio_machdep.c` first; the interface survey ranked `counter.h` second.
   All three were wrong in a way that reading the files fixed in ten minutes.
3. **Make the gate fail before trusting it.** Every checker in `tools/` that
   this session added was run against a deliberately broken input first.
4. **A skipped step is not a passing step.** "Boot it" has reported `skipped`
   in every run and it would be easy to read a run summary as a boot.
5. **Landing needs attribution.** Until a build is green, a failure after a
   change cannot be blamed on the change.

---

## 1. Blocked on the first boot

PBSD has never been started. `tools/ci/boot_test.py` exists, distinguishes
four outcomes, and has been skipped in every run because the build failed
first. Nothing below in this section should land before it runs once.

- [ ] **Boot once.** The blocker chain was: SafeStack runtime → wrong triple
      → aliasing. The current `memstick` run is the first past the point that
      killed runs 8, 9 and 11.
- [ ] **Adopt `sys/sys/atomic_generic.h`.** Written, 350 lines, replaces
      5,107 across six headers; 23 of 24 (arch, width) cells verified
      lock-free. i386 keeps 44 lines for its 64-bit block. Adoption is six
      `#include` edits plus deleting what they replace.
- [ ] **Adopt `_stdint_generic.h` / `_inttypes_generic.h`.** Written, 1,266
      macro expansions verified identical on all six targets. `SIG_ATOMIC_*`
      stays per-architecture: about 180 lines each becomes 3.
- [ ] **First `lib/msun` `.c` → `.cpp` port.** `CXXSTD=c++23` and the
      mirrored `CXXFLAGS` are in place; 88 of 120 verify under the flags that
      ship. One rename plus one `COMMON_SRCS` line.
- [ ] **`counter.h`**, four implementations not one — generic covers arm,
      arm64, riscv, powerpc; amd64 keeps `zpcpu_add` (a `lock` prefix on a
      hot counter is not free) and i386 keeps the CX8 path.
- [ ] **`uio_machdep.c`**, two implementations not one: `pmap_map_io_transient`
      for architectures with a direct map, `sf_buf_alloc` for those without.
      Two machine-independent files and a way to choose. A hot VM path, so
      this one wants a boot *and* a stress test, not just a compile.

## 2. Correctness and hardening, found and not yet closed

- [ ] **Re-measure the assembly numbers.** `WITHOUT_MACHDEP_OPTIMIZATIONS`
      was a no-op for `lib/msun` on amd64 and i386 — every one of the 23 and
      47 `ARCH_SRCS` entries collided on object name with a `COMMON_SRCS`
      `.c`, and which one bmake built was decided by suffix-rule precedence.
      Fixed; `docs/ASSEMBLY.md`'s msun figures were describing an option that
      was not taking effect and need re-measuring against a build.
- [ ] **SafeStack and CFI for arm, powerpc, riscv.** `lib/libclang_rt` builds
      those runtimes for amd64, arm64 and i386 only. `src.conf.pbsd` now says
      so instead of asking for a mitigation that cannot link. Closing the gap
      means building compiler-rt for the other three, which is a real port,
      not a configuration line.
- [ ] **The three `lib/libc` ports that are IR-equal and not ABI-equal.**
      37 against 34, unchanged by the four header guards already added. The
      oracle now names them and the divergent symbols; the fix is one
      `__BEGIN_DECLS` per header once they are named.
- [ ] **Kernel-config parity beyond `HARDENEDBSD`.** The gate checks one
      config per architecture. There are 21 others with HardenedBSD in the
      name or ASLR in their purpose — `HARDENEDBSD-CORE`, `-MINIMAL`,
      `-NODEBUG`, `-UP`, `-MMCCAM`, `HARDENEDBSD64`, `BEAGLEBONE-HARDENEDBSD`,
      `RPI2-HARDENEDBSD`, `QEMU-HARDENEDBSD`, `LATT-SEC`, `LATT-ASLR`. Each
      can drift the same way riscv's did. `--conf` already exists; the gate
      needs to loop.
- [ ] **`sys/x86` shared code on i386.** `hwpstate_intel.c` used
      `pc_small_core`, which only amd64's `struct pcpu` has. That family
      directory is shared by two architectures with different `pcpu`
      layouts and nothing checks that a `sys/x86` file compiles for both
      except building i386 — which now happens, so this is closed by the
      matrix rather than by a tool. Worth a note in `docs/PORTABILITY.md`.
- [ ] **`vm_mmap.c`'s `MAP_32BIT` path on 32-bit.** Now randomised as an
      ordinary mmap rather than out of a delta that does not exist there.
      Correct, and untested at runtime — a boot with `PAX_ASLR` and a
      `MAP_32BIT` mmap is what proves it.

## 3. Consolidation, ranked by what is measured

`tools/arch_interface.py` ranks by interface agreement, `tools/arch_clusters.py`
by how many implementations there really are. Both are in CI as reports.

- [ ] **`busdma_machdep.c`** — 2,458 lines, 4 architectures, 3 clusters
      (`arm64,riscv` | `arm` | `powerpc`). The largest unexamined candidate.
- [ ] **`_bus.h`** — 174 lines across six for four typedefs that differ only
      in width. `<stdint.h>`-shaped.
- [ ] **`kdb.h`** (371 lines, agreement 0.60), **`iodev.h`** (154, 0.67),
      **`db_machdep.h`** (570, 0.31) — read before scoring, per rule 2.
- [ ] **`stand/ficl/<arch>/sysdep.{c,h}`** — six copies. `sysdep.c` is two
      texts (aarch64/arm/powerpc/riscv identical; amd64/i386 identical);
      `sysdep.h` is 85–92% identical pairwise. In the boot loader, so it
      wants a boot behind it.
- [ ] **`vmparam.h`** — 1,695 lines, agreement 0.27 with 8 generators. Some
      of it is genuinely per-MMU; the shared part has not been separated.
- [ ] **Do not bother with** `pmap.c` (5 copies, 5 implementations, saves 0),
      `pmap.h`, `cpu.h`, `pte.h`. Three independent measurements agree.
- [ ] **`linux_systrace_args.c` / `linux_proto.h` / `linux_sysent.c`** —
      9,032 + 2,111 + 470 lines of apparent duplication, all generated from
      `syscalls.master`. Consolidating the output is working on the wrong
      end; regenerating from one description is the real change.

## 4. Assembly

340 `.S` files under `sys`, 179 under `lib`, 37 under `stand`, 294 under
`contrib` (not ours). 230 files under `sys` and 59 under `lib` contain
`__asm`.

- [ ] **Inline assembly is the larger half and has never been inventoried
      properly.** `asm_inventory.py` counts `.S` files. The atomics work
      showed the interesting assembly is inline in headers — 145 sites in
      `atomic.h` alone — and a `.S` count says nothing about it.
- [ ] **Classify the remaining `sys/*.S` by what it does**, the way
      `asm_removal_plan.py` did for `lib/libc`. Trap vectors, context
      switch, cache maintenance and early boot are irreducible; string and
      checksum routines are not.
- [ ] **`stand/` assembly (37 files)** is boot-loader entry code and mostly
      irreducible, but it has not been looked at at all.

## 5. The C++ port

- [ ] **The 88 is not a rate.** `lib/msun` verifies at 73% with 0.22 edits
      per file; `lib/libc` at 31% with 8.8. Any scope-wide claim needs the
      corpus-bias correction stated with it.
- [ ] **Widen the oracle beyond `lib/msun` and `lib/libc`.** `bin`,
      `usr.bin`, `sbin` have never been measured. The first attempt ran past
      the 90-minute job limit and reported nothing; it needs to be split per
      scope rather than made bigger.
- [ ] **74 `pbsd/` modules on disk are not in `CMakeLists.txt`** because
      nothing has verified them. `check_pbsd_modules.py` reports the number;
      nothing reduces it.
- [ ] **149 modules fail the Linux build** and are held by a ratchet. Every
      one is a FreeBSD private header that Linux cannot supply. A FreeBSD
      runner would move that number to its real value, whatever that is.
- [ ] **`pbsd/userland`** carries 2,146 files with the path encoded in the
      filename. That scheme has to be undone before any of it can be built
      in place.
- [ ] **`pbsd.fp` has customers waiting**: the five copies of NetBSD's
      `softfloat.h` (two distinct texts) are what `basic_float<E,M>`
      replaces, and `bfloat64` needs `uint_for` widened past 64 bits.

## 6. Toolchain and build

- [ ] **The `llvm21` port's triple.** Aliasing its runtime directory works
      and is a CI-environment fix. The durable answer is either building
      `lib/libclang_rt` with the external toolchain, or asking the port for
      `-print-runtime-dir` and passing `-resource-dir` — neither tried.
- [ ] **`TOOLCHAIN=internal` has not been built since the untracking.** The
      guard fails fast with the re-fetch command, which is the right
      behaviour and is not the same as knowing it works.
- [ ] **No release artifact has ever been produced.** `memstick` and `iso`
      exist as stages; neither has completed.
- [ ] **Reproducible builds.** `MK_REPRODUCIBLE_BUILD` is on and nothing
      checks that two builds of the same tree agree.
- [ ] **`WITHOUT_KERBEROS` remains deliberately unset** — 2,179 files, and
      turning it off is a product decision about what PBSD can do.

## 7. CI and the gates

Fourteen gates exist. What is missing is not more gates but coverage of the
things that have actually broken.

- [x] ~~A gate that would have caught the msun `ARCH_SRCS` bug.~~
      `tools/machdep_option_check.py`. Checks two things that are not the
      same: the collision, read out of the source lists (70 machine-dependent
      sources in `lib/msun` share an object name with a machine-independent
      one — 23 on amd64, 47 on i387), and the mechanism, that each of the
      four consumers still contains the text that makes the option take
      effect and that no fifth has appeared without one. Verified to fail in
      both directions.
- [ ] **The matrix is `workflow_dispatch` plus weekly.** Six architectures
      per push is too expensive; on the default branch after a `sys/` change
      is not.
- [ ] **No gate on the reporters themselves.** `show_hardening.sh` was
      rewritten five times. Its parse is now tested inline; the test is not
      run anywhere.
- [ ] **The Linux ratchet counts failures and not their reasons.** 149 is a
      number; the reason histogram exists in the report and is not asserted.
- [ ] **Boot-test outcomes are not ratcheted.** Once it boots, "it booted
      last time" should be a floor like the oracle's.

## 8. Reach: what a seventh architecture would cost today

The honest measure of all of the above. Today a new architecture needs:
`atomic.h` (800+ lines of lock-free assembly), `_stdint.h` and `_inttypes.h`
(390), `counter.h`, `_bus.h`, `pmap.h`/`pte.h`/`vmparam.h` (irreducible),
`uio_machdep.c`, `busdma_machdep.c`, a `sysdep.h` for the loader, and a
kernel config. The generic headers written but not adopted remove the first
two outright.

- [ ] **Pick a seventh and try it**, even without hardware. `powerpc64le`
      and `armv6` are in `KNOWN_ARCHES` and are not built. A cross-build of
      one of them is the cheapest possible test of every claim in this file.

## 9. Security posture

- [ ] **The hardening table is per-option, not per-binary.** Nothing checks
      that shipped binaries actually carry PIE, RELRO, BIND_NOW and a stack
      protector — `readelf` on the staged tree would.
- [ ] **PAX behaviour is untested at runtime.** Every PAX option is on and
      none has been exercised; the assertions PAX relies on are what
      `INVARIANTS` provides, and that was accidentally removed from five
      architectures earlier today and only found by a build.
- [ ] **`HBSD_RESIST_FINGERPRINTING`, `HARDEN_KLD`, `PAX_SEGVGUARD`** are set
      in `std.hardenedbsd` and unverified in a running system.
- [ ] **No fuzzing, no sanitizer build.** `MK_ASAN` and `MK_UBSAN` are
      reported by `show_hardening.sh` and always `no`.

## 10. Deliberately not done, with the reason

- **`WITHOUT_KERBEROS`** — functionality, not fat.
- **`RETPOLINE` outside x86** — `-mretpoline` is rejected by clang elsewhere;
  the equivalent is different hardware.
- **Hoisting `uio_machdep.c` to one file** — it is two implementations and
  hoisting means breaking three architectures.
- **A generic `counter.h` that uses `atomic_add_64` everywhere** — a `lock`
  prefix on amd64's hottest counters.
- **Asserting 8-byte alignment for i386 64-bit atomics** — it does produce
  `CMPXCHG8B`, and it is a promise about the caller's object that a header
  cannot make.
- **`SCNX*` macros** — `%X` is a printf conversion; C99 gives scanf no such
  thing.
