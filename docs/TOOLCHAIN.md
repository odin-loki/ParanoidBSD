# The toolchain, in-tree or external

`contrib/llvm-project` is **14,337 files, 290 MB, 13% of the tracked tree** —
the single largest thing in the repository. Building it is the largest single
cost in a `buildworld`.

PBSD already maintains it separately: `lib/clang/include/llvm/Support/VCSRevision.h`
records `llvmorg-21.1.8`, and HardenedBSD 15-stable is on 19.x. The 6,846
`contrib/llvm-project` files that differ from upstream are PBSD's own upgrade.
So the tree is already carrying the maintenance cost of a separate LLVM
without getting the benefit of treating it as one.

## The two configurations

```sh
# external (default): builds against packaged clang21, world ships no compiler
sh tools/ci/build_boot_image.sh world

# internal: builds clang from contrib/llvm-project, self-hosting
TOOLCHAIN=internal sh tools/ci/build_boot_image.sh world
```

External sets `CROSS_TOOLCHAIN=hbsd/toolchains/llvm21.mk` and adds
`hbsd/src.conf.pbsd-external-toolchain`, which sets `WITHOUT_TOOLCHAIN`.
`share/mk/src.opts.mk:544` cascades that to `MK_CLANG`, `MK_LLD`, `MK_LLDB`
and `MK_LLVM_BINUTILS`.

## What it costs

**Self-hosting.** The installed system cannot rebuild itself without
installing a compiler first. For a general-purpose OS that is a significant
property; for a hardened appliance-style system it is arguably a feature,
since a compiler on the running system is attack surface.

External is the default, and that is a deliberate trade rather than an
oversight. The arguments for self-hosting are real and worth stating so the
choice can be revisited:

- **Supply chain.** Building the compiler from readable source rather than
  trusting a binary package. For a hardening-focused system, trusting-trust
  is not a theoretical concern.
- **Reproducibility.** The compiler that built a release is pinned in the
  tree; a package repository can change or yank a version.
- **Patchability.** The compiler could be patched for the OS - hardening
  defaults, SafeStack, CFI.
- **Bootstrap from nothing.** Clone plus a running FreeBSD, no network.

Set against that: PBSD was not using any of them. contrib/llvm-project is
carried at llvmorg-21.1.8 with no PBSD patches to the compiler itself, so the
patchability argument is unexercised; and building a compiler in order to
build a system that does not run one is most of buildworld's time.

`TOOLCHAIN=internal` still works and is one word, so a release that needs the
self-hosted property can have it.

## What it buys

- 14,337 files stop being compiled to produce a system that does not run them.
- An LLVM upgrade becomes a package version bump instead of a merge against
  6,846 modified files, which is what makes the vendor branch in
  `docs/VENDOR.md` awkward today.
- The build gets materially shorter. `buildworld` spends most of its time in
  `cross-tools` and `lib/clang`.

## Keeping the two in step

The compiler comes from a package and the headers still come from
`contrib/llvm-project`. Different major versions build most of the way and
then fail in compiler-rt, the sanitizer runtimes, or libc++ ABI details.

```sh
python3 tools/check_toolchain_version.py
```

reads the major out of `VCSRevision.h` and out of the toolchain file and
asserts they match. It runs as a boot-image precondition.

## The compiler sources are not tracked

Done. `llvm/`, `lldb/` and `clang/` (except `clang/lib/Headers`) are out of
git — **9,772 files, 236 MB** — with re-fetch instructions in `.gitignore`.

What stays, and why each one has to:

| subtree | files | why |
|---|---:|---|
| `compiler-rt` | 1,372 | `lib/libcompiler_rt`, `lib/libclang_rt`, `stand/libsa`, `stand/i386/boot2` and `lib/libc/arm/aeabi` all compile source out of it |
| `libcxx` | 1,917 | `lib/libc++` |
| `libc` | 628 | referenced by the build |
| `lld` | 235 | linker sources |
| `clang/lib/Headers` | 275 | `sys/modules/aesni` and `sys/modules/blake2` `.PATH` into it for intrinsics |
| `openmp` | 103 | `lib/libomp` |
| `libunwind` | 33 | `lib/libexecinfo` includes its headers |

This is the part that makes "untrack contrib/llvm-project" wrong as stated.
Most of that directory is not the compiler — it is the runtime the world links
against no matter what compiled it, plus, in two cases, source files the
kernel and the boot loader compile directly. Removing the lot would break the
build with an external toolchain just as thoroughly as with an internal one.

`TOOLCHAIN=internal` now fails immediately with the re-fetch command rather
than discovering the absence several minutes into a build, and
`check_clang_srcs.py` reports that there is nothing to check instead of
reporting 3,855 missing sources.

### The earlier note, kept because it was the right question

The repository already has this pattern — `.gitignore` carries

```
# HardenedBSD ports tree (huge / re-clonable)
/hbsd/ports/
```

with a re-fetch line. `contrib/llvm-project` is the same shape of thing: huge,
re-clonable from `llvm/llvm-project` at the tag `VCSRevision.h` names, and
needed at build time rather than being PBSD's own work.

It should not happen until an external-toolchain `buildworld` is green,
because until then the in-tree sources are the only thing that builds. And it
would need `vendor/hardenedbsd` updated in the same change, or the vendor
branch would carry files the mainline does not.
