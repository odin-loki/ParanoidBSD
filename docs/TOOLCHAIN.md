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
# in-tree (default): builds clang, ships clang, self-hosting
sh tools/ci/build_boot_image.sh world

# external: builds against packaged clang21, world ships no compiler
TOOLCHAIN=external sh tools/ci/build_boot_image.sh world
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

This is why external is not the default. It is a property to give up
deliberately, not to discover you have given up.

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

## If the sources should stop being tracked

Not done, and listed here because it is the obvious next question.

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
