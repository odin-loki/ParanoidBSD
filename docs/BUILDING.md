# Building PBSD

`hbsd/src` is a complete HardenedBSD source tree — 107,357 files, 31,887 of
them under `sys/`, with the HardenedBSD kernel configs intact. It builds.

## Before anything else

The tree was committed with **every file mode 100644**. FreeBSD's build runs
its own scripts directly, so it stopped at the first one it tried to exec:

```
contrib/bmake/boot-strap: contrib/bmake/configure: Permission denied
```

That is fixed, and guarded:

```sh
python3 tools/check_exec_bits.py
```

A text file starting with `#!` must be executable; a binary file must not be.
Run it after any bulk import — the same mistake will recur the next time the
tree is copied in rather than merged.

Two more checks belong to the same family, because the flat import lost more
than modes:

```sh
python3 tools/check_vendor_manifest.py
python3 tools/check_subdirs.py hbsd/src --fail-on-missing
```

## What the import ate

Against `hardened/15-stable/main`, 4,243 upstream files are absent here. Most
of that is deliberate — `sys/arm`, `sys/powerpc`, `sys/riscv` and their
build glue are gone on purpose, and `contrib/llvm-project` differs because
upstream has moved to a newer LLVM. But 124 files were lost by accident, in
two distinct ways, and both stopped the build.

**Ignore rules matched real source.** `hbsd/src/.gitignore` is upstream's own
file, and it lists `*.o`, `*.a`, `*.so`, `*.po`. Upstream tracks files those
rules match — it added them before the rules existed. Git never un-tracks a
file when a rule starts matching it, so upstream keeps them; a fresh
`git add` over a copied tree does not. 114 files went that way, including
every prebuilt Highpoint RAID blob:

```
make[4]: don't know how to make sys/dev/hpt27xx/amd64-elf.hpt27xx_lib.o. Stop
```

They are restored with `git add -f`, listed in `tools/vendor_manifest.txt`,
and `tools/check_vendor_manifest.py` asserts every one is still tracked and
still on disk.

**A directory vanished but its `SUBDIR` entry did not.** `usr.bin/Makefile`
line 57 still reads `id \`, and `usr.bin/id` was not there. The likely cause
is line 32 of `hbsd/src/.gitignore` — `ID`, meant for a GNU id-utils tag
file — matching the directory `usr.bin/id` on a case-insensitive filesystem
during the import. `contrib/netbsd-tests/usr.bin/id` went with it.

`include/Makefile` was the same shape and worse, because it is not
hypothetical for this target:

```make
.if ${MACHINE_CPUARCH} == "amd64"
SUBDIR+=		i386
```

An amd64 build installs the i386 headers, and `include/i386` was absent.
`tools/check_subdirs.py` now walks every `Makefile` in the tree and checks
that each plain `SUBDIR` token names a directory that exists — 2,632 entries,
and it reports the file and line, so the answer arrives in seconds rather
than hours into a build.

Both checks run as preconditions in `pbsd-boot-image.yml`, before the VM
starts.

## On FreeBSD (the supported path)

```sh
sh tools/ci/build_boot_image.sh kernel      # ~tens of minutes
sh tools/ci/build_boot_image.sh world       # hours
sh tools/ci/build_boot_image.sh memstick    # bootable USB image
sh tools/ci/build_boot_image.sh iso         # bootable installer ISO
```

Knobs: `KERNCONF` (default `HARDENEDBSD`), `TARGET`/`TARGET_ARCH` (default
`amd64`), `OBJ` (default `/usr/obj/pbsd`), `JOBS`.

Available kernel configs: `HARDENEDBSD`, `HARDENEDBSD-CORE`,
`HARDENEDBSD-MINIMAL`, `HARDENEDBSD-NODEBUG`, plus FreeBSD's `GENERIC`,
`MINIMAL` and the `LINT` variants.

The stages are a ladder on purpose. `kernel` proves the config compiles;
`world` proves the userland does; only then is an image worth the hours.

## In CI

`.github/workflows/pbsd-boot-image.yml`, manual dispatch, pick a stage and a
kernel config. It runs on a FreeBSD VM and uploads `.img`/`.iso` as an
artifact. Start at `kernel` with `HARDENEDBSD-MINIMAL`.

## Cross-building from Linux

`tools/build/make.py` is FreeBSD's own Linux/macOS entry point and does work
for world and kernel:

```sh
cd hbsd/src
sudo apt-get install -y bmake flex bison time libarchive-dev
MAKEOBJDIRPREFIX=/tmp/pbsd-obj \
XCC=/usr/bin/clang XCXX=/usr/bin/clang++ \
XCPP=/usr/lib/llvm-18/bin/clang-cpp XLD=/usr/bin/ld.lld \
python3 tools/build/make.py --host-bindir=/usr/bin -j"$(nproc)" \
    TARGET=amd64 TARGET_ARCH=amd64 kernel-toolchain
```

Verified to reach *stage 1.2, bootstrap tools* on Ubuntu 24.04 with clang 18
— bmake bootstraps and the legacy shims stage completes. The image targets
(`makefs`, `mkimg`) still want a FreeBSD host, so use this to check the tree
compiles and the VM to produce something bootable.

## Why the modes were missing

`hbsd/src` arrived in a single commit with no remote, no submodule and no
vendor branch: a flat copy with upstream history and file modes discarded.
Re-importing it as a vendor branch would have carried the modes, would not
have run the ignore rules over upstream's own tracked files, and would make
future upstream a `git merge` rather than another copy. Every accident on
this page has the same single cause. Worth doing before the next sync.
