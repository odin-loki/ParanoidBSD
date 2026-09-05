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

`world` is the next rung and is hours rather than minutes. (Superseded:
run 20 built a memstick and booted its kernel — see below.)

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

**So: PBSD's loader runs.** That was further than any previous run had got
and it is not a boot. (Whether the kernel starts was open at this point;
run 20, below, answers it.)

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

### Run 20: the kernel boots

```
---<<BOOT>>---
Copyright (c) 2013-2025 The HardenedBSD Project.
FreeBSD 15.1-STABLE-HBSD  HARDENEDBSD amd64
clang version 21.1.8
HardenedBSD: initialize and check features
             (__HardenedBSD_version 1500001 __FreeBSD_version 1501501).
CPU: QEMU Virtual CPU version 2.5+ (2300.36-MHz K8-class CPU)
real memory  = 2147483648 (2048 MB)
avail memory = 2039971840 (1945 MB)
...
vtblk0: <VirtIO Block Adapter> on virtio_pci0
vtblk0: 1122MB (2298747 512 byte sectors)
uart0: <16550 or compatible> port 0x3f8-0x3ff irq 4 flags 0x10 on acpi0
uart0: console (115200,n,8,1)
Trying to mount root from ufs:/dev/ufs/HardenedBSD_Install [ro,noatime]...
Dual Console: Serial Primary, Video Secondary
```

**PBSD's kernel starts.** It initialises HardenedBSD's feature checks,
probes the virtio disk the image is on, takes the serial port as its
console, and mounts root.

Then six minutes and fifty-one seconds of nothing, and the run times out
at 420s. No panic, no trap, no `mountroot>`.

### Where it stopped, exactly

The last line is not a guess about how far it got. `Dual Console: Serial
Primary, Video Secondary` is printed by `start_init()` in
`sys/kern/init_main.c:765`, and the lines around it are:

```c
	vfs_mountroot();
	/* Wipe GELI passphrase from the environment. */
	kern_unsetenv("kern.geom.eli.passphrase");
	/* For Multicons, report which console is primary to both */
	if (boothowto & RB_MULTIPLE) {
		if (boothowto & RB_SERIAL)
			printf("Dual Console: Serial Primary, Video Secondary
");
	...
	while ((path = strsep(&tmp_init_path, ":")) != NULL) {
		...
		/* Now try to exec the program. */
```

So `vfs_mountroot()` **returned** — the root filesystem is mounted — and
the next thing the kernel does is exec `/sbin/init`. That is where it
stopped.

(The message itself only prints because the console steering sets
`RB_MULTIPLE`; an unsteered boot would go straight from mountroot to the
exec loop. It is a marker of the steering, not a symptom.)

**So: the kernel boots and root mounts, and the first userland process
produces nothing.** Whether `init` is failing to exec, or execs and hangs
before printing, is the next question and is not answered yet.

### Run 21: the exec is reached

Same image, `loader_set = boot_verbose=YES boot_single=YES`:

```
Dual Console: Serial Primary, Video Secondary
start_init: trying /sbin/init
<420s of nothing>
```

`start_init: trying %s` is printed by the exec loop immediately before
`kern_execve()`, so **the kernel reaches the exec of `/sbin/init`**. What
did *not* appear is as informative:

* no `exec /sbin/init: error N` — the loop prints that for any failure
  other than `ENOENT`;
* no second `start_init: trying` — `ENOENT` would have moved to the next
  path in `init_path`;
* no `panic: no init` — that is what running out of paths does;
* no `Enter full pathname of shell`, though `boot_single=YES` was set.

So `kern_execve()` did not return an error, did not fall through, and did
not produce a running program that said anything.

### The gap that run exposed in the test

A shell exec'd as PID 1 prints no banner. Nothing in the `USERLAND`
pattern list would match a working `/rescue/sh`, so the *success* case of
the next experiment would have been recorded as a failure.

`--poke-after` closes it: once the kernel has started and nothing new has
been printed for N seconds, the test types `echo __PBSD_ALIVE__` at the
console, up to three times, and `__PBSD_ALIVE__` is a `USERLAND` pattern.
A marker echoed back proves a shell is there; silence after three tries is
evidence there is not. Tested both ways against a fake that boots to a
silent-but-echoing shell and one that is genuinely dead.

### Run 22: the experiment did not run

`loader_set = boot_verbose=YES init_path=/rescue/sh`, and the kernel said:

```
start_init: trying /sbin/init
<three pokes, no marker, 420s>
```

`/sbin/init`, not `/rescue/sh`. So `boot_verbose` took effect — that line
only prints under `bootverbose` — and `init_path` did not. The
discriminator never ran, and the three unanswered pokes are evidence about
a boot that was never modified.

`init_path` should work: `start_init()` reads it with
`kern_getenv("init_path")`, and the loader's `md_copyenv()` copies every
variable that is not flagged `EV_NOKENV` into the kernel environment. Which
of *never typed*, *typed and dropped*, or *arrived and ignored* this was
could not be read out of the log — with `boot_verbose` on, the boot is
hundreds of lines and the steering is above all of them.

So the tool records it. Every loader command is now confirmed the same way
the console command already was (echo, or a fresh `OK ` prompt), and the
list prints at the **end**, where a tail reaches it:

```
    loader commands:
      ok   set boot_verbose="YES"   (confirmed by echo)
      ok   set init_path="/rescue/sh"   (confirmed by echo)
      ok   set console="comconsole,vidconsole"   (confirmed by echo)
```

That is the same lesson as the oracle's SUMMARY block, in a second place:
the fact worth reading was being written where nothing could read it.

### Run 23: the command landed, the marker did not come back

`loader_set = init_path=/rescue/sh:/sbin/init`, `boot_verbose` off so the
log stays short:

```
    loader commands:
      ok   set init_path="/rescue/sh:/sbin/init"   (confirmed by echo)
      ok   set console="comconsole,vidconsole"   (confirmed by echo)
    phases reached: loader, kernel
```

Two facts, and one boundary on them.

* **The loader received `init_path` this time.** Run 22's identical
  attempt did not take effect; run 23's was echoed back in full. The
  difference is that run 22 sent its two extras with a fixed sleep and no
  receipt, and this run waits for each one — so "typed and dropped" is the
  best explanation of run 22, and the receipt logic is what makes the
  difference visible either way.
* **Three pokes, no `__PBSD_ALIVE__`.** With a colon list beginning at the
  statically linked `/rescue/sh` — no `init`, no `rtld` — the console
  answered nothing.

**The boundary:** the receipt is from the *loader*, not the kernel. With
`boot_verbose` off, `start_init: trying <path>` never prints, so this run
does not show that the kernel actually tried `/rescue/sh`. The loader
accepted the variable; whether it reached `kern_getenv("init_path")` is
unobserved.

So the honest reading is *narrower* than "a static shell also fails to
run": it is that setting `init_path` to a static shell changed nothing
observable. The run that closes the gap is both together —
`boot_verbose=YES init_path=/rescue/sh:/sbin/init` — where
`start_init: trying /rescue/sh` would confirm the kernel used it.

### Run 25: a loader-prompt `set` does not reach the kernel

Both together, and the loader confirmed all three commands:

```
    loader commands:
      ok   set boot_verbose="YES"   (confirmed by echo)
      ok   set init_path="/rescue/sh:/sbin/init"   (confirmed by echo)
      ok   set console="comconsole,vidconsole"   (confirmed by echo)
```

and the kernel printed:

```
start_init: trying /sbin/init
```

**`/sbin/init`.** The loader echoed the command back in full and the
kernel used the compiled-in default anyway. So the receipt proves the
loader *received* the line and nothing more — exactly the boundary run 23
was written up under, now demonstrated rather than argued.

Three runs went into learning that a `set` at the interactive prompt is
not a way to configure the kernel. Why it is not is unknown and is not
guessed at here.

### Configure the image instead

`build_boot_image.sh` takes `LOADER_CONF_EXTRA` (workflow input
`loader_conf`, semicolons separating lines) and appends it to
`/boot/loader.conf` **inside the built image** — `mdconfig` to attach,
`gpart` to find the filesystem, mount, append, unmount, detach.

**Run 26 found the layout was not what I assumed.** The first version
looked only for a GPT `freebsd-ufs` partition, and the memstick is MBR
with a BSD label:

```
FAIL no freebsd-ufs partition in the image:
=>      1  2302010    md0  MBR  (1.1G)
        1   131050  md0s1  efi  (64M)
   131051  2170960  md0s2  freebsd  [active]  (1.0G)
```

There is no `freebsd-ufs` at the top level at all; the filesystem is
`md0s2a`, one level down inside the slice. The stub test that passed
beforehand used a GPT layout I had made up — the same "tested against my
idea of the thing" failure as the boot-menu pattern, except that this
time the defensive branch turned it into one clear line naming the real
layout instead of a corrupted image.

It now tries GPT `freebsd-ufs` first, then descends into a `freebsd` MBR
slice, and falls back to the `a` partition by convention if `gpart`
cannot read the label. Both selections verified against the **observed**
output, not an invented one. The append is checked and read back, because
`makefs` sizes these images to fit and a setting that silently did not
land is exactly the failure this mechanism exists to prevent. An empty
value leaves the image untouched and never runs `mdconfig`.

The value travels to the FreeBSD VM as a file in the workspace rather than
a `${{ }}` expansion, for the same reason as `loader_set`: an expansion is
pasted into the shell before it runs.

### Run 27: `loader.conf` does not carry `init_path` either

The injection worked. `gpart` found the MBR slice, descended into it,
mounted `md0s2a`, appended, and read the file back:

```
== appending to /boot/loader.conf in the image
   init_path="/rescue/sh:/sbin/init"
   attached as /dev/md0
   MBR: descending into slice md0s2
   UFS filesystem: /dev/md0s2a
   /boot/loader.conf now ends:
     vfs.mountroot.timeout="10"
     kernels_autodetect="NO"
     loader_menu_multi_user_prompt="Installer"
     init_path="/rescue/sh:/sbin/init"
   detached /dev/md0
```

and the kernel printed `start_init: trying /sbin/init`. Again. The
setting was in the file, in the image the kernel booted, and
`kern_getenv("init_path")` returned nothing.

**The premise this route was chosen on was wrong.** The claim was that
`loader.conf` reaches the kernel because the root mount comes from
`vfs.root.mountfrom` in it. Neither half holds:

* `release/Makefile` writes exactly three lines into that file
  (`vfs.mountroot.timeout`, `kernels_autodetect`,
  `loader_menu_multi_user_prompt`) and `vfs.root.mountfrom` is not one
  of them. It is `grep`-able; it was not `grep`-ed.
* The loader sets that variable itself, in `stand/common/boot.c:384`,
  from the device it loaded the kernel off. It is in the loader
  environment because the loader put it there, not because a file did.

So the root mount says the loader's *own* environment reaches
`kern_getenv()`. It says nothing about whether `loader.conf` gets into
that environment, and two runs now say `init_path` does not.

### What `loader.conf` does reach

`sys/kern/subr_boot.c` has a table, `howto_names`, of twelve variables:

```
boot_askname boot_cdrom  boot_ddb    boot_dfltroot
boot_gdb     boot_multicons boot_mute boot_mutemsgs
boot_pause   boot_serial boot_single boot_verbose
```

`boot_env_to_howto()` turns them into `boothowto` bits, and on this
platform it is called by **the loader** — `bi_getboothowto()` in
`stand/efi/loader/bootinfo.c` — before the kernel is entered. The bits
travel in the bootinfo block, not in the kernel environment. That is why
`boot_verbose` has worked every time since run 22 while `init_path`
never has: they do not use the same mechanism, and only one of them has
been demonstrated.

That makes `boot_single` the next experiment, because it is on the list
that works and it answers a bigger question than `init_path` did.

### The observation `init_path` was chasing

`start_init()` prints `start_init: trying <path>` and then calls
`kern_execve()`. Read what the loop actually does with the result:

```c
error = kern_execve(td, &args, NULL, oldvmspace);
if (error == EJUSTRETURN) { ...; return; }      /* success */
if (error != ENOENT) printf("exec %s: error %d\n", path, error);
```

`ENOENT` would print the *next* `trying` line. Any other error would
print `exec /sbin/init: error N`. Neither happened, so exactly two things
are possible: `kern_execve()` never returned, or it returned
`EJUSTRETURN` — **and init is running.**

One `trying` line followed by silence is what a *successful* exec looks
like from the kernel's side. It has been read here as a hang for five
runs, and it is equally consistent with init(8) being alive and
producing no output.

`boot_single="YES"` is what separates them, and **run 21 already set
it** — at the loader prompt, alongside `boot_verbose`, and the log shows
the loader echoing both back:

```
  [loader: set boot_verbose="YES"]
set boot_verbose="YES"
OK   [loader: set boot_single="YES"]
set boot_single="YES"
OK
```

Single-user init execs a shell on `/dev/console` instead of running
`rc`, and `sbin/init/Makefile` builds with `-DDEBUGSHELL -DSECURE`, so it
prints `Enter full pathname of shell or RETURN for /bin/sh:` before it
does. That string is in `boot_test.py`'s `USERLAND` list. Nothing
appeared.

So the evidence points at init *not* being alive — and it is evidence,
not proof, for the reason this page keeps running into. Run 21's
`boot_single` was echoed by the loader; whether it reached `boothowto` is
unobserved, exactly as `init_path`'s three receipts were before run 25
showed an echoed `set` changing nothing. The inference is stronger here,
because `boot_verbose` travelled the identical road in that same run and
did take effect. It is not the same as having seen it.

What has not been tried is `boot_single` in **`loader.conf`** rather than
at the prompt. Run 25 is the whole reason that distinction is worth a
run: an echoed prompt `set` and a line in the file are not the same
delivery, and one of them has already been caught not arriving.

So run 30 puts all three in `loader.conf` together — `boot_single`,
`boot_verbose`, `init_path` — and nothing at the loader prompt but the
console. Three readings from one boot:

* does `start_init: trying` print at all? `boot_verbose` comes only from
  the file this time, so that answers whether `loader.conf` reaches the
  loader environment.
* does it say `/rescue/sh` or `/sbin/init`? That is `kern_getenv()`, for
  the fifth time and by the only means left.
* does the single-user prompt appear? Run 21 asked this through the
  prompt; this asks it through the file.

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
