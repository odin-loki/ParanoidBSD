# Building PBSD

`hbsd/src` is a complete HardenedBSD source tree — 107,357 files, 31,887 of
them under `sys/`, with the HardenedBSD kernel configs intact. It builds.

## Status

```
>>> Kernel(s)  HARDENEDBSD-MINIMAL built in 525 seconds, ncpu: 4, make -j4
```

`pbsd-boot-image.yml`, run 3, 2026-09-05. The first kernel this repository
has produced. It took four attempts and each one failed on a different piece
of the flat import:

| Attempt | Failed on |
|---|---|
| 1 | `contrib/bmake/configure: Permission denied` — no executable bits |
| 2 | `realpath: sys/i386/include: No such file` — `kmod.mk` symlinks i386 for amd64 |
| 3 | `don't know how to make sys/dev/hpt27xx/amd64-elf.hpt27xx_lib.o` — eaten by `*.o` |
| 4 | built |

`world` is the next rung and is hours rather than minutes. Nothing has
produced a bootable image yet.

## The ladder now ends in a boot

```
kernel    buildkernel                     tens of minutes
world     buildworld buildkernel          about half an hour
vm        ... + vm.raw        + BOOT + ASK
memstick  ... + memstick.img  + BOOT      the image, and QEMU starting it
iso       ... + disc1.iso     + BOOT
```

`memstick` and `iso` are the installer. They boot to bsdinstall's menu,
which proves the kernel and userland start and is as far as it goes — there
is nobody to talk to. `vm` builds `release/vm.raw`, which has a root login on
the serial console, and that is the difference between "it booted" and "and
here is what it says about itself".

Everything up to `world` proves the tree compiles. `tools/ci/boot_test.py`
is the only thing in the repository that proves the result runs: it starts the
image under QEMU, watches the serial console, and decides from what it says.

  * a login prompt, a welcome banner or `Starting local daemons` — it booted;
  * `panic:`, `Fatal trap`, or a `mountroot>` prompt — it did not, and the
    matching line is printed rather than summarised;
  * kernel banner but nothing after it — a hang *after* boot, reported as a
    different failure from never starting;
  * silence — the loader never ran and the image is not bootable at all.

It runs on the Linux runner rather than in the FreeBSD VM. The image is built
on FreeBSD because `release/` wants a FreeBSD host; booting it is just QEMU
and does not. The boot log is uploaded whether the boot passed or failed,
because a failed boot is exactly when it is wanted.

### Run 15 reached it, and the answer was wrong

Run 15 built a memstick, collected it, and ran the boot test, which said:

```
booting out/memstick.img (1178 MB) as amd64
--- 4s, log in out/boot.log
OK  booted: reached userland
```

**It had not.** Four seconds in, the console was here:

```
FreeBSD/x86 bootstrap loader, Revision 3.0
Loading /boot/defaults/loader.conf
Loading /boot/loader.conf
... Welcome to HardenedBSD ...          <- the loader MENU
```

and the log contains no `Copyright (c) 1992`, no `real memory  =` and no
trademark line. The kernel had not been loaded. The tool matched

```python
re.compile(rb"Welcome to (Paranoid|Hardened|Free)BSD")
```

against the title of the boot loader's own menu and called it userland.

That is the same defect this project has now hit six times — a check that
returns something which parses as an answer to a question it was not asking
— and this is the most consequential instance of it, because the wrong
answer was "PBSD boots".

`boot_test.py` now tracks three phases separately and only the third is a
boot:

| phase | what it proves | markers |
|---|---|---|
| `loader` | `/boot/loader` ran | bootstrap loader banner, `Loading /boot/...`, the menu title |
| `kernel` | the kernel started | `Copyright (c) 199x-`, `real memory =`, `avail memory =`, `Timecounter "`, the trademark line |
| `userland` | init ran | `login:`, `Starting local daemons`, `bsdinstall`, `Enter full pathname of shell` |

The verdict is the furthest phase reached, and there is a distinct failure
message for each of "the loader ran and the kernel never started", "the
kernel started and never reached userland", and "nothing at all".

Verified by replaying **run 15's actual console bytes** through the fixed
tool: it reports `phases reached: loader` and fails with *the loader menu
drawing itself is not a boot*. A synthetic full boot still passes, a
kernel-that-hangs is reported as a hang, and a panic still wins over
everything.

**So: PBSD's loader runs.** That is further than any previous run got and it
is not a boot. Whether the kernel starts is still open.

### The console the loader hands the kernel

The loader writes to the serial port. The kernel it starts does not,
because the menu says

```
5. Cons: Video
```

and `console` is what the loader puts in kenv. So everything from
`Copyright (c) 1992` onwards goes to a VGA device that `-nographic` is not
showing anyone, and a run left alone sits through its whole timeout seeing
nothing after the menu — which reads as "the kernel never started" and is
not evidence of that at all.

`boot_test.py` therefore drives the menu: `3` to escape to the loader
prompt, `set console="comconsole,vidconsole"`, `boot`.

### Run 16: the loader got fifteen characters of a forty-character command

```
11:01:37  [loader prompt: console=comconsole,vidconsole, boot]
11:08:33  set console="co
```

Seven minutes apart, and nothing in between. The 16550 the loader reads has
a sixteen-byte FIFO and no flow control, and the loader polls it between
other work, so writing the whole line in one `write()` overran it and the
rest was dropped silently. The command never terminated, `boot` was never
reached, and the run spent its full 420 seconds at a prompt.

The verdict it printed said *the console was set … so it is the kernel not
printing*. That was false: it asserted a step the log shows did not happen.

Two fixes. `_send()` types one byte at a time with a 15 ms gap — forty
characters take 0.6 s and no FIFO is troubled. And steering now waits for a
**receipt** before sending `boot`: either the echo of the tail of the
command, or a fresh `OK ` prompt. Without one it boots anyway and the
verdict says the console was *not confirmed* and that this is not evidence
about the kernel either way.

**Still open, and now for a reason that is understood.** PBSD's kernel has
never been asked to start over a console anyone was watching. Runs 14 and
15 failed on the image path, run 16 on the UART. The next memstick run is
the first that can answer the question.

## Asking the system about itself

`--run NAME=CMD` logs in after a successful boot and runs commands, writing
each one's output to `NAME.txt` beside the boot log. On a `vm` image the
workflow asks for `uname -a`, `sysctl hardening`, every setuid and setgid
file in the image, and `kldstat -v`, and then
`tools/hardening_sysctls.py --check` compares the sysctl dump against the
defaults extracted from the source.

That is the only way to answer several questions this repository has been
carrying:

* `show_hardening.sh` says `PAX_ASLR` is compiled in. Whether
  `hardening.pax.aslr.status` *defaults* to opt-out is a different question
  and it decides whether the mitigation applies to anything.
* `tools/setuid_inventory.py` reads Makefiles, so it says what the tree
  declares. `find / -perm -4000` says what the image contains, and the two
  can differ.

The mechanics were tested against a stand-in console before being pointed at
a real image — a script that prints a plausible boot, offers a login and
hands over to a shell. It caught three bugs that a real boot would have hit:
the log being closed before the interrogation ran, waiting for a `login:`
prompt that was already on screen and had already been consumed by the boot
loop, and keying on how many times a sentinel appears — which works on a
serial console because a tty echoes, and fails anywhere that does not. That
last one is the important one: counting occurrences would have made the
harness and production behave differently, which is the opposite of what a
harness is for. Paired begin/end markers work in both.

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
