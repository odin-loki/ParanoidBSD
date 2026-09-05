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

---

# Part two: the sixty per cent nobody has opened

Everything above is adjacent to something this work has touched. This part is
the rest of the tree, surveyed rather than worked, because that is where the
unknown unknowns are. File counts are tracked files under `hbsd/src`.

| area | files | opened |
|---|---:|---|
| `sys` | 33,302 | partly |
| `contrib` | 25,472 | only `llvm-project` |
| `crypto` | 11,426 | **no** |
| `lib` | 5,610 | `libc`, `msun`, `libmd` only |
| `share` | 3,834 | `share/mk` only |
| `usr.bin` | 3,312 | **no** |
| `usr.sbin` | 2,855 | **no** |
| `tests` | 2,400 | **no** |
| `cddl` | 2,232 | **no** |
| `sbin` | 1,280 | **no** |
| `secure` | 1,158 | **no** |
| `bin` | 1,146 | **no** |
| `stand` | 755 | `ficl` only |
| `libexec` | 577 | **no** |
| `release` | 284 | **no** |

## 11. The CI harness the tree already ships

`tests/ci/` is FreeBSD's own, and it does what this project has been building
by hand:

```
ci-smoke   build a release image and boot it
ci-full    ... then run the whole Kyua suite inside it and extract reports
```

with `Makefile.amd64`, `.aarch64`, `.armv7`, `.powerpc64`, `.powerpc64le`,
`.riscv64`, `USE_QEMU=1` by default, bhyve on amd64, and per-architecture
firmware installed for you — `sysutils/opensbi` and
`sysutils/u-boot-qemu-riscv64` for riscv, and the equivalents elsewhere.

`tools/ci/boot_test.py` is a hand-written subset of `ci-smoke`. This is the
same shape as every other finding in this work: the mechanism was already
there.

- [ ] **Run `ci-smoke` instead of, or alongside, `boot_test.py`.** The four
      outcomes `boot_test.py` distinguishes are better reporting than
      `ci-smoke` gives, so the honest answer is probably to drive `ci-smoke`
      and keep the log parser.
- [ ] **`ci-full` on amd64.** It is the only way the 2,400-file test suite
      ever runs.
- [ ] **`ci-smoke` for arm64, riscv, powerpc.** The arch matrix builds
      kernels; this boots them. `powerpc64le` and `armv6` have makefiles and
      no kernel config in PBSD.
- [ ] **Reconcile with `pbsd-boot-image.yml`.** Two image-building paths in
      one repository is one too many.

## 12. The test suite nobody runs

2,400 files under `tests/`, including `tests/sys/mac`, `tests/sys/file`,
`tests/sys/net`, `tests/etc/rc.d`. `contrib/atf` and `contrib/kyua` and
`usr.bin/kyua` are all present.

`hbsd/src.conf.pbsd` sets `WITHOUT_TESTS=YES`, which removes them from the
*image* — correct, and it has never been the reason they do not run. Nothing
runs them.

- [ ] **A green Kyua run is the first real statement that PBSD works.**
      Everything so far says it compiles.
- [ ] **`tests/sys/mac` specifically.** PBSD turns on the MAC framework and
      PAX; there is a MAC test suite in the tree and it has never been
      executed against this kernel.
- [ ] **Ratchet the pass count** the way the IR oracle is ratcheted.
- [ ] **The C++ ports have no runtime tests at all.** IR equivalence is a
      statement about the compiler's output, not about behaviour under load.
      `pbsd/tests/` has two harnesses and neither is wired to anything.

## 13. Trust and supply chain

- [ ] **`secure/caroot/trusted/` holds 120 root CAs and
      `blacklisted/` holds none.** PBSD ships stock FreeBSD's trust set. For
      a system whose name is Paranoid, which 120 certificate authorities are
      trusted by default is a product decision that has not been made.
- [ ] **No SBOM.** `tools/check_vendor_manifest.py` tracks 114 ignore-eaten
      files; that is not a bill of materials. `contrib/` is 25,472 files
      from dozens of upstreams with no manifest of versions or licences.
- [ ] **`MK_REPRODUCIBLE_BUILD` is on and unverified.** Two builds of the
      same tree have never been compared. That is a `cmp -s` over the staged
      trees plus a diffoscope run on what differs.
- [ ] **No release signing.** Nothing in `release/` signs an image, and
      nothing verifies one.
- [ ] **The untracked `contrib/llvm-project` re-fetch is pinned to
      `llvmorg-21.1.8` by tag, not by hash.** A tag can move.

## 14. Verified boot, which the kernel already supports

`sys/security/mac_veriexec/` and `sys/dev/veriexec/` are in the tree with
SHA-1, SHA-256 and SHA-512 backends. `std.hardenedbsd` does not enable them.

- [ ] **Decide whether PBSD has a verified-boot story.** A hardened OS with
      no integrity chain from firmware to userland is a gap that
      `PAX_ASLR` does not fill.
- [ ] **`MAC_VERIEXEC` + `MAC_VERIEXEC_SHA256` in `std.hardenedbsd`**, and a
      manifest-generation step in `release/`.
- [ ] **EFI Secure Boot / loader signing.** Nothing found under `stand/`.
- [ ] **`HARDEN_KLD` is on and untested.** It forbids module loading after
      boot; nothing checks that it does.

## 15. The 59 hardening sysctls

`grep -rhoE 'hardening\.[a-z_.0-9]+' sys/` yields **59 distinct knobs** —
`hardening.pax.aslr.*`, `hardening.pax.mprotect.status`,
`hardening.harden_rtld`, `hardening.harden_shm`, `hardening.harden_tty`,
`hardening.forbid_kmod`, `hardening.kmalloc_zero`, `hardening.elf_pie_only`,
`hardening.control.acl.status`, and the rest.

`tools/ci/show_hardening.sh` reports **build options**. Not one of these 59
runtime defaults is checked by anything.

- [ ] **Dump all 59 on a booted image and assert the defaults.** The kernel
      option being compiled in is not the same as the knob defaulting to on
      — the same distinction that made `WITHOUT_MACHDEP_OPTIMIZATIONS` inert.
- [ ] **Ratchet them.** A default that silently relaxes across an upstream
      merge is exactly the drift `std.hardenedbsd` was written to stop, one
      layer down.
- [ ] **Document which are PBSD policy and which are HardenedBSD defaults.**

## 16. Userland: 8,600 files, unexamined

`bin` 1,146, `sbin` 1,280, `usr.bin` 3,312, `usr.sbin` 2,855, `libexec` 577.

- [ ] **The oracle has never been pointed at any of it.** `lib/msun` and
      `lib/libc` are the only measured scopes. `bin` is small, self-contained
      and the natural third.
- [ ] **`WITHOUT_*` options for what a hardened system does not ship.**
      `src.conf.pbsd` turns off tests, games, docs, examples and OFED. It has
      never been asked what else a paranoid system has no business shipping —
      `sendmail`, `telnet`, `rsh`, `tftp`, `finger`, `talk`, `bsnmp`, `ppp`,
      `slattach`. Each is a `WITHOUT_` line and an attack-surface argument.
- [ ] **setuid inventory.** Nothing enumerates what ships setuid or setgid.
      That is one `find` over the staged tree and it is the single most
      useful hardening report a BSD can produce.

## 17. `libexec/rtld-elf` — the highest-value target in userland

8 C files, 7,154 lines in `rtld.c` alone. Every dynamically linked program
in the system runs it before `main`. HardenedBSD has `hardening.harden_rtld`
and PBSD has never looked at the file.

- [ ] **Read it against the hardening options.** RELRO and BIND_NOW are
      linker flags; the loader is what enforces them.
- [ ] **`LD_*` environment handling** is the classic local-privilege-
      escalation surface in every Unix.
- [ ] It is also a strong C++ port candidate: self-contained, no external
      dependencies, heavily exercised by every test.

## 18. Release engineering: ten targets, none built

`release/` has `Makefile.vm`, `.ec2`, `.gce`, `.azure`, `.oci`, `.vagrant`,
`.firecracker`, `.mirrors`. `sys/amd64/conf/FIRECRACKER` exists.

- [ ] **Produce one artifact of any kind.** No image has ever been built.
- [ ] **`FIRECRACKER` is a hardened-workload shape** — minimal kernel, fast
      boot, no legacy devices — and it already has a config.
- [ ] **`MINIMAL` and `HARDENEDBSD-MINIMAL` exist** and are unbuilt. A
      minimal kernel is a smaller attack surface and a much faster matrix.

## 19. `crypto/` and `secure/` — 12,584 files

OpenSSL, OpenSSH, and the libraries that make TLS work. Never examined.

- [ ] **Which OpenSSL, and how far behind?** `check_toolchain_version.py`
      pins LLVM; nothing pins or reports this.
- [ ] **OpenSSH configuration defaults.** `secure/ssh.mk` exists.
- [ ] **`WITHOUT_OPENSSL_KTLS`, cipher policy, FIPS-shaped questions** are
      all `src.conf` decisions nobody has made.

## 20. `cddl/` — ZFS, 2,232 files

- [ ] **ZFS is on and untested.** `tests/sys/cddl` exists.
- [ ] **Encryption-at-rest defaults** — a paranoid system's installer should
      have an opinion, and there is no installer work at all yet.

## 21. Performance: traded away, never measured

The stated position is "I don't care if it costs some performance." That is a
decision, and it is currently unquantified in both directions.

- [ ] **Microbenchmark the MI C against the MD assembly** for the functions
      `WITHOUT_MACHDEP_OPTIMIZATIONS` switches — `memcpy`, `strlen`, `fmod`,
      `sqrt`, the SHA and MD5 blocks. One harness, six architectures, and the
      number stops being a guess. Note this cannot be measured honestly until
      the msun option bug fix lands, because on x86 the option was inert.
- [ ] **Measure the generic atomics against the hand-written ones.** Same
      instructions in most cells; the counter path is where a `lock` prefix
      would show.
- [ ] **Boot time and kernel size** across the six configs.
- [ ] **The cost of the hardening itself** — PIE, RELRO, BIND_NOW, SSP,
      SafeStack, CFI. A hardened system should be able to say what its
      mitigations cost.

## 22. Documentation and the option surface

- [ ] **`share/man/man5/src.conf.5` is 2,066 lines** and documents the
      upstream options. PBSD's own `src.conf.pbsd` is documented only in its
      comments and in `docs/`.
- [ ] **No man page describes PBSD.** No `pbsd(7)`, no `hardening(7)`.
- [ ] **`UPDATING-HardenedBSD` exists and PBSD has no UPDATING.**
- [ ] **The docs are eleven files and growing** — `ASSEMBLY`, `BUILDING`,
      `CONSOLIDATION`, `PORTABILITY`, `TOOLCHAIN`, `VENDOR`, `FLOAT`,
      `ROADMAP`, plus `migration/`. There is no index and no statement of
      what PBSD *is* in the repository itself.

## 23. Ports and packages

- [ ] **`hbsd/ports` is untracked** and there is no statement of what a PBSD
      package set would be.
- [ ] **The external toolchain comes from a package**, so PBSD already has a
      hard dependency on a ports tree it does not track or pin.
- [ ] **No `pkg` repository, no signing key, no update path.** An OS that
      cannot be updated is a demo.

## 24. What "first class" should mean, written down

The phrase drives most of the decisions in this file and is not defined
anywhere. A candidate contract, so it can be argued with:

> An architecture is first class when its `HARDENEDBSD` kernel builds, its
> world builds, its image boots to a login prompt, the Kyua suite passes at
> the same ratchet as amd64, every hardening option that its hardware can
> support is on and verified at runtime, and `tools/arch_contract.py` reports
> no missing header or source.

Today **six architectures meet the first clause and none meets the third.**

- [ ] **Encode that contract as a gate**, one row per architecture, and let
      the table be the project's status page.
