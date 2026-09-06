#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Compile hbsd/src on a Linux host, against FreeBSD's OWN headers.

The first attempt to run any analyser over this tree on Linux produced 32
diagnostics on 12 files and every one was noise:

    typedef redefinition with different types ('__int32_t' (aka 'int') vs 'long')

That is FreeBSD's <sys/_types.h> and glibc's meeting in one translation
unit. It is why the IR oracle passes -Wno-everything and compares IR
instead of diagnostics: identical noise on both sides cancels. For a model
checker nothing cancels - CBMC could not build 91 of 120 translation
units, and every one of those was this.

The fix is not a FreeBSD host and not a sysroot download. The headers are
IN THE TREE - hbsd/src/include is FreeBSD's userland header set and
hbsd/src/sys/sys is its kernel one. -nostdinc removes glibc entirely and
the tree becomes self-hosting, with two things supplied from outside:

  machine/, x86/   sys/<arch>/include is what <machine/foo.h> means, and
                   the build normally arranges that with a symlink. A
                   temporary directory of symlinks does the same here, and
                   it is what makes this arch-parametric rather than
                   amd64-only.
  clang's own      stddef.h, stdarg.h, limits.h, float.h and the intrinsic
                   headers belong to the COMPILER, not the libc, and
                   -nostdinc drops them too. -print-resource-dir names
                   where they are.

Verified on lib/libc/string/strcat.c, stdlib/abs.c, quad/muldi3.c and
stdlib/atoi.c: zero errors, where the same files could not be compiled at
all against glibc.
"""

from __future__ import annotations

import functools
import os
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# TARGET_ARCH -> (sys/<dir>/include for machine/, extra sys dirs)
ARCH = {
    "amd64":     ("amd64",   ["x86"]),
    "aarch64":   ("arm64",   []),
    "armv7":     ("arm",     []),
    "i386":      ("i386",    ["x86"]),
    "powerpc64": ("powerpc", []),
    "riscv64":   ("riscv",   []),
}

# clang's name for each architecture. Nothing was passing one of these,
# so clang took its target from the HOST, and every architecture in this
# sweep was checked with x86-64's data model. That is not a detail:
#
#   sys/arm/arm/mp_machdep.c has
#       CTASSERT(PAGE_SIZE % sizeof(struct pcpu) == 0);
#   and -Xclang -fdump-record-layouts says why it failed --
#
#       0 | struct pcpu
#       0 |   struct thread * pc_curthread
#       8 |   struct thread * pc_idlethread     <- 8-byte pointers
#
#   on armv7, a 32-bit architecture. sizeof(struct pcpu) came out 640
#   with no --target and 512 with an armv7 one; 4096 % 512 == 0, and the
#   assertion the tree ships is correct.
#
# arm and i386 are ILP32 and were being compiled LP64: every struct
# layout, every pointer, every long. arm64/riscv64/powerpc64 are LP64
# like the host, so they only ACCIDENTALLY agreed -- and powerpc64 is
# big-endian and was being checked little-endian.
TRIPLE = {
    "amd64":     "x86_64-unknown-freebsd15.0",
    "aarch64":   "aarch64-unknown-freebsd15.0",
    "armv7":     "armv7-unknown-freebsd15.0",
    "i386":      "i386-unknown-freebsd15.0",
    "powerpc64": "powerpc64-unknown-freebsd15.0",
    "riscv64":   "riscv64-unknown-freebsd15.0",
}

# goto-cc is a gcc driver. It rejects --target= outright, and --arm-linux
# and friends are "uninterpreted gcc option"s; -m32 is the one thing it
# takes, and it takes it properly - the goto model's sizeof(void *) goes
# from /*8l*/ to /*4*/ and __i386__ replaces __x86_64__.
#
# So the model checker can be pointed at exactly two data models, the
# host's and 32-bit x86, and these two architectures are the ones -m32
# describes:
#
#   i386   exactly - it IS 32-bit x86.
#   armv7  in every respect CBMC reasons about - ILP32, little-endian,
#          the same integer widths. What it does not get right is the
#          predefined macro, so an `#ifdef __arm__` in an armv7 file
#          still takes its false branch under goto-cc.
#
# The remaining three (aarch64, riscv64, powerpc64) get the host model,
# which is right for the first two and WRONG FOR POWERPC64: it is
# big-endian and goto-cc cannot be told so. Endianness-dependent results
# on powerpc64 are therefore not sound, and that limit is the compiler's,
# not something a flag here can close.
GOTO_ILP32 = frozenset({"i386", "armv7"})


def target_flags(arch: str, cc: str = "clang") -> list[str]:
    """Point the compiler at ARCH, in whichever spelling CC understands."""
    if "goto-" in Path(cc).name:
        return ["-m32"] if arch in GOTO_ILP32 else []
    t = TRIPLE.get(arch)
    return [f"--target={t}"] if t else []


# libc's per-architecture private header directory is named differently
# from the kernel's: lib/libc/amd64, lib/libc/aarch64, ...
LIBC_ARCH = {
    "amd64": "amd64", "aarch64": "aarch64", "armv7": "arm",
    "i386": "i386", "powerpc64": "powerpc64", "riscv64": "riscv",
}


@functools.lru_cache(maxsize=None)
def resource_dir(cc: str = "clang") -> str:
    try:
        p = subprocess.run([cc, "-print-resource-dir"],
                           capture_output=True, text=True, timeout=30)
        if p.returncode == 0:
            return (Path(p.stdout.strip()) / "include").as_posix()
    except OSError:
        pass
    return ""


@functools.lru_cache(maxsize=None)
def _lhdrs() -> tuple[str, ...]:
    """include/Makefile's LHDRS: top-level names that live in sys/sys.

    FreeBSD does not keep <errno.h> in include/. include/Makefile:46 has

        LHDRS=  aio.h errno.h fcntl.h linker_set.h poll.h stdatomic.h
                stdint.h syslog.h ucontext.h

    and the header install SYMLINKS each from sys/sys/. 293 translation
    units failed on errno.h alone before this was read, and the answer was
    never a missing file - it was a build step this shim was not doing.

    Parsed rather than copied, so it tracks the Makefile.
    """
    mk = SRC / "include" / "Makefile"
    try:
        text = re.sub(r"\\\n", " ", mk.read_text(errors="replace"))
    except OSError:
        return ()
    for line in text.splitlines():
        if line.startswith("LHDRS"):
            return tuple(t for t in line.split("=", 1)[1].split()
                         if t.endswith(".h"))
    return ()


@functools.lru_cache(maxsize=None)
def machine_shim(arch: str = "amd64") -> str:
    """A directory laid out the way the installed header tree is.

    machine/ and x86/ from sys/<arch>/include, the LHDRS names from
    sys/sys, float.h from the arch directory (include/Makefile's MHDRS),
    and math.h/complex.h/fenv.h from lib/msun - which is where they live,
    since msun installs its own.
    """
    sysdir, extras = ARCH.get(arch, ARCH["amd64"])
    d = Path(tempfile.mkdtemp(prefix=f"pbsd_inc_{arch}_"))

    def link(name: str, target: Path) -> None:
        if target.exists() and not (d / name).exists():
            (d / name).symlink_to(target, target_is_directory=target.is_dir())

    inc = SRC / "sys" / sysdir / "include"
    link("machine", inc)
    for e in extras:
        link(e, SRC / "sys" / e / "include")

    for h in _lhdrs():
        link(h, SRC / "sys" / "sys" / h)

    # MHDRS: float.h and floatingpoint.h are machine-dependent.
    for h in ("float.h", "floatingpoint.h"):
        link(h, inc / h)

    # lib/msun installs these itself.
    for h in ("math.h", "complex.h"):
        link(h, SRC / "lib" / "msun" / "src" / h)
    msun_arch = {"amd64": "x86", "i386": "x86", "aarch64": "aarch64",
                 "armv7": "arm", "powerpc64": "powerpc",
                 "riscv64": "riscv"}.get(arch, "x86")
    link("fenv.h", SRC / "lib" / "msun" / msun_arch / "fenv.h")
    return d.as_posix()


ARCH_DIR = {"amd64": "amd64", "aarch64": "aarch64", "arm": "armv7",
            "i386": "i386", "powerpc": "powerpc64", "powerpc64": "powerpc64",
            "riscv": "riscv64", "powerpcspe": "powerpc64"}


# The KERNEL's per-architecture directory names, which are a third
# spelling again: sys/arm64 where libc says aarch64 and the ARCH table's
# key is aarch64. sys/<this> -> the ARCH key to build the shim from.
SYS_ARCH = {"amd64": "amd64", "arm64": "aarch64", "arm": "armv7",
            "i386": "i386", "powerpc": "powerpc64", "riscv": "riscv64"}


def arch_of(rel: str, default: str = "amd64") -> str:
    """A source under lib/libc/<arch>/, lib/msun/<arch>/ or sys/<arch>/.

    Verifying lib/libc/aarch64/gen/getcontextx.c against amd64's
    <machine/*.h> is not a check of anything; it is a different program.

    That was written for lib/ and the kernel was left out, so every file
    under sys/arm64, sys/arm, sys/powerpc and sys/riscv - 671 of them - got
    amd64's machine/ shim. Most failed to compile, which is the harmless
    outcome; the danger is the ones that DO compile, because then the tool
    reports a clean check of a program that does not exist.
    """
    parts = rel.split("/")
    if len(parts) > 2 and parts[0] == "lib" and parts[1] in ("libc", "msun"):
        cand = ARCH_DIR.get(parts[2])
        if cand:
            return cand
    if len(parts) > 2 and parts[0] == "sys":
        cand = SYS_ARCH.get(parts[1])
        if cand:
            return cand
    return default


@functools.lru_cache(maxsize=None)
def iface_shim() -> str:
    """The kernel interface headers, GENERATED the way the real build does.

    2,540 of the 6,345 translation units the sweep could not compile failed
    on one line:

        fatal error: 'device_if.h' file not found

    and 369 more on vnode_if.h. Those files are not missing and they are not
    optional - they do not exist in a source tree at all. sys/kern/device_if.m
    is an interface DESCRIPTION, and buildkernel runs

        awk -f sys/tools/makeobjops.awk sys/kern/device_if.m -h

    to write device_if.h into the object directory. Same for every other
    *_if.m (138 of them: bus_if, cpufreq_if, pci_if, ...) and, through a
    different generator, sys/kern/vnode_if.src -> vnode_if.h.

    Generating them is not a stub or an approximation: it is the same awk
    script on the same input the kernel build uses, so the declarations are
    the ones the kernel actually compiles against. Skipping it silently cost
    46 percent of the kernel corpus - the analyser and the model checker both
    reported those files as ERROR, and an ERROR is not a finding, so the
    unchecked files simply did not appear anywhere in the results.
    """
    d = Path(tempfile.mkdtemp(prefix="pbsd_iface_"))
    tools = SRC / "sys" / "tools"
    mko, vno = tools / "makeobjops.awk", tools / "vnode_if.awk"
    if not mko.is_file():
        return d.as_posix()

    for m in (SRC / "sys").rglob("*_if.m"):
        try:
            subprocess.run(["awk", "-f", str(mko), str(m), "-h"],
                           cwd=d, check=True, capture_output=True, timeout=60)
        except (OSError, subprocess.SubprocessError):
            # One interface that will not generate is one interface's worth
            # of files still failing, not a reason to lose the other 137.
            pass

    # vnode_if.awk is run THREE times by the build, not once:
    #
    #   sys/conf/kern.post.mk:499   -h   vnode_if.h
    #   sys/conf/kern.post.mk:501   -p   vnode_if_newproto.h
    #   sys/conf/kern.post.mk:503   -q   vnode_if_typedef.h
    #
    # and vnode_if.h's first line includes vnode_if_typedef.h. Generating
    # only -h therefore resolved <sys/vnode.h> one step further and then
    # failed on the next, which is every kernel file that includes
    # sys/mount.h or sys/vnode.h - most of the file systems.
    # The same story for the device-ID tables. sys/conf/kmod.mk:510 and
    # its neighbours run four more generators over four more description
    # files, and every USB driver includes "usbdevs.h".
    #
    #   usbdevs2h.awk    sys/dev/usb/usbdevs        -h, -d
    #   sdiodevs2h.awk   sys/dev/sdio/sdiodevs      -h, -d
    #   miidevs2h.awk    sys/dev/mii/miidevs        (no flag)
    #   acpi_quirks2h.awk sys/dev/acpica/acpi_quirks (no flag)
    for tool, inp, flags in (
            ("usbdevs2h.awk", "dev/usb/usbdevs", ("-h", "-d")),
            ("sdiodevs2h.awk", "dev/sdio/sdiodevs", ("-h", "-d")),
            ("miidevs2h.awk", "dev/mii/miidevs", ()),
            ("acpi_quirks2h.awk", "dev/acpica/acpi_quirks", ())):
        t, i = tools / tool, SRC / "sys" / inp
        if not (t.is_file() and i.is_file()):
            continue
        for flag in (flags or ("",)):
            cmd = ["awk", "-f", str(t), str(i)] + ([flag] if flag else [])
            try:
                subprocess.run(cmd, cwd=d, check=True, capture_output=True,
                               timeout=60)
            except (OSError, subprocess.SubprocessError):
                pass

    src = SRC / "sys" / "kern" / "vnode_if.src"
    if vno.is_file() and src.is_file():
        for flag in ("-h", "-p", "-q"):
            try:
                subprocess.run(["awk", "-f", str(vno), str(src), flag],
                               cwd=d, check=True, capture_output=True,
                               timeout=60)
            except (OSError, subprocess.SubprocessError):
                pass
    return d.as_posix()


@functools.lru_cache(maxsize=None)
def kernconf_options(config: str = "HARDENEDBSD",
                     arch: str = "amd64") -> tuple[tuple[str, str], ...]:
    """The options a real kernel configuration sets, as config(8) reads them.

    The empty-opt_*.h shim (below) is a real configuration and the
    conservative one: code under an unset option is not checked rather
    than checked wrongly. But it is a configuration NOBODY SHIPS. With
    every option off, INET is off, and a sweep of sys/netinet is checking
    a TCP stack compiled without IP.

    Measured on sys/netinet/tcp_syncache.c: 2 findings with nothing
    defined, 1 with -DINET, and 7 with -DINET -DINET6 - because the third
    is the only one where most of the file is compiled at all. The empty
    shim was not producing conservative results, it was producing results
    about a different program.

    So the option set comes from the kernel configuration PBSD builds.
    config(8)'s rules, followed rather than approximated:

      sys/amd64/conf/HARDENEDBSD   `include GENERIC`, `options FOO`,
                                   `options FOO=value`, `nooptions FOO`
      sys/conf/options             NAME -> the header it lands in; a name
      sys/conf/options.<arch>      with no header goes to opt_<name>.h

    Returns ((NAME, value), ...) with value "" for a plain option.
    """
    # `arch` is the ARCH-table key - aarch64, armv7, powerpc64, riscv64 -
    # and the kernel's directory is named differently: sys/arm64,
    # sys/arm, sys/powerpc, sys/riscv. Passing the key straight through
    # pointed at sys/aarch64/conf, which does not exist, and every
    # architecture but amd64 and i386 (where the two names happen to
    # coincide) silently got NO options at all - the empty shim again, on
    # four of six, with nothing saying so.
    #
    # HARDENEDBSD exists under all six.
    sysdir = ARCH.get(arch, ARCH["amd64"])[0]
    confdir = SRC / "sys" / sysdir / "conf"
    opts: dict[str, str] = {}
    seen: set[str] = set()

    def read(name: str) -> None:
        name = name.strip().strip('"')
        if name in seen:
            return
        seen.add(name)
        for cand in (confdir / name, SRC / "sys" / "conf" / name):
            if not cand.is_file():
                continue
            for raw in cand.read_text(errors="replace").splitlines():
                line = raw.split("#", 1)[0].strip()
                if not line:
                    continue
                head, _, rest = line.partition("\t")
                parts = line.split()
                if parts[0] == "include" and len(parts) > 1:
                    read(parts[1])
                elif parts[0] == "options" and len(parts) > 1:
                    for tok in parts[1:]:
                        nm, _, val = tok.partition("=")
                        opts[nm] = val
                elif parts[0] == "nooptions" and len(parts) > 1:
                    for tok in parts[1:]:
                        opts.pop(tok.partition("=")[0], None)
            return

    read(config)
    return tuple(sorted(opts.items()))


@functools.lru_cache(maxsize=None)
def option_headers(arch: str = "amd64") -> tuple[tuple[str, str], ...]:
    """sys/conf/options: which opt_*.h each option name lands in."""
    out: dict[str, str] = {}
    sysdir = ARCH.get(arch, ARCH["amd64"])[0]
    for f in (SRC / "sys" / "conf" / "options",
              SRC / "sys" / "conf" / f"options.{sysdir}"):
        if not f.is_file():
            continue
        for raw in f.read_text(errors="replace").splitlines():
            line = raw.split("#", 1)[0].strip()
            if not line:
                continue
            parts = line.split()
            name = parts[0]
            out[name] = (parts[1] if len(parts) > 1
                         else f"opt_{name.lower()}.h")
    return tuple(sorted(out.items()))


@functools.lru_cache(maxsize=None)
def opt_shim(arch: str = "amd64") -> str:
    """Empty opt_*.h, which is exactly what config(8) writes for an unset option.

    A kernel source says `#include "opt_inet.h"` and config(8) generates
    that file during buildkernel: it holds `#define INET 1` when the option
    is in the kernel configuration and IS EMPTY when it is not. There is no
    such file in a source tree.

    So an empty one is not a stub standing in for something real - it is
    the genuine content for a kernel built without that option, and it
    makes every `#ifdef INET` take its false branch. That is a real
    configuration, and it is the conservative one: the code under the
    option is not checked rather than checked wrongly.

    Names are harvested from the tree rather than listed, so this does not
    go stale.
    """
    d = Path(tempfile.mkdtemp(prefix="pbsd_opt_"))
    names = set()
    pat = re.compile(r'#\s*include\s+"(opt_[A-Za-z0-9_]+\.h)"')
    sysdir = SRC / "sys"
    for f in sysdir.rglob("*.[ch]"):
        try:
            names.update(pat.findall(f.read_text(errors="replace")))
        except OSError:
            pass
    # Which configuration this is.
    #
    # PBSD_KERNCONF=none keeps every header empty - the original behaviour,
    # and still the right answer when you want to know what survives with
    # nothing turned on. Anything else names a kernel config under
    # sys/<arch>/conf, and the default is the one PBSD actually builds.
    #
    # config(8)'s rules, followed: an option in sys/conf/options maps to a
    # header, `options FOO=v` writes the value, and opt_dontuse.h is
    # config(8)'s sink for options that only steer `files` rules - it
    # writes no header for those, so neither does this.
    conf = os.environ.get("PBSD_KERNCONF", "HARDENEDBSD")
    setopts = {} if conf == "none" else dict(kernconf_options(conf, arch))
    # Options that add INSTRUMENTATION rather than behaviour, and that the
    # checker cannot parse.
    #
    # KDTRACE_HOOKS turns on the SDT probes, and sys/sys/sdt.h:218 writes
    # them as `asm goto(...)`. clang's analyser gives up on it: with
    # KDTRACE_HOOKS on, sys/netinet went to 85 errors of 105 translation
    # units, and 39 of them are that one macro.
    #
    # Dropping them is not the same kind of choice as leaving a real
    # option off. A dtrace probe does not change what the surrounding code
    # computes - it is a nop sled the kernel patches at runtime - so the
    # properties being checked are the same either way. Each name here has
    # to meet that test.
    for instrumentation in ("KDTRACE_HOOKS", "KDTRACE_MIB_SDT",
                            "HWPMC_HOOKS", "EXTERR_STRINGS"):
        setopts.pop(instrumentation, None)
    hdrof = dict(option_headers(arch))
    per: dict[str, list[str]] = {}
    for name, val in setopts.items():
        h = hdrof.get(name, f"opt_{name.lower()}.h")
        if h == "opt_dontuse.h":
            continue
        per.setdefault(h, []).append(
            f"#define\t{name}\t{val}" if val else f"#define\t{name}\t1")

    names.update(per)
    for n in names:
        body = per.get(n)
        if body:
            (d / n).write_text(
                f"/* {n}: from {conf}, via sys/conf/options */\n"
                + "\n".join(sorted(body)) + "\n")
        else:
            (d / n).write_text(f"/* {n}: not set in this configuration */\n")

    # clang's own <limits.h> ends in `#include_next <limits.h>`, and under
    # -nostdinc there is nowhere for that to go: 293 kernel translation
    # units failed on "no include path in which to search for limits.h".
    # The kernel's is sys/sys/limits.h, which is what the real build
    # resolves <limits.h> to inside sys/. Placed here so it is found
    # BEFORE the compiler's.
    lim = SRC / "sys" / "sys" / "limits.h"
    if lim.is_file():
        (d / "limits.h").symlink_to(lim)
    return d.as_posix()


# Some kernel sources do not compile with the standard flag set, and the
# tree says so per-file rather than by convention: sys/conf/files* carries
#
#   contrib/ck/src/ck_epoch.c  standard compile-with "${NORMAL_C} -I$S/contrib/ck/include"
#
# for 402 of them. Guessing prefixes would be inventing an answer the
# build system already gives, so this reads config(8)'s own input the way
# kernconf_options() reads its options -- $S is sys/.
_CONF_FLAG = re.compile(r'-I\$S/([^\s"]+)|(-D[A-Za-z_][^\s"]*)')


@functools.lru_cache(maxsize=None)
def conf_file_includes(arch: str = "amd64") -> dict:
    """sys-relative source path -> the extra flags sys/conf/files gives it."""
    sysdir = ARCH.get(arch, ARCH["amd64"])[0]
    names = ["files", f"files.{sysdir}"] + [f"files.{e}" for e in ARCH.get(arch, ARCH["amd64"])[1]]
    out: dict[str, list[str]] = {}
    for name in names:
        f = SRC / "sys" / "conf" / name
        if not f.exists():
            continue
        # Logical lines: a trailing backslash continues onto the next.
        text = f.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            line = line.strip()
            if not line or line.startswith("#") or "compile-with" not in line:
                continue
            src = line.split()[0]
            if not src.endswith((".c", ".cc", ".cpp")):
                continue
            flags = []
            for inc, define in _CONF_FLAG.findall(line):
                if inc:
                    flags.append(f"-I{SRC}/sys/{inc.rstrip('/')}")
                elif define:
                    flags.append(define)
            if flags:
                out.setdefault(src, []).extend(flags)
    return {k: tuple(dict.fromkeys(v)) for k, v in out.items()}


# What sys/conf/files cannot say, because these trees are built ONLY as
# modules and a module Makefile is where their flags live. Each entry is
# a path prefix (sys-relative) and the Makefile line it was copied from.
MODULE_INCLUDES = (
    # sys/conf/kmod.mk:114-117. Every linuxkpi consumer gets this set;
    # the wireless drivers under contrib/dev are all linuxkpi ports and
    # their own Makefiles add `CFLAGS+= ${LINUXKPI_INCLUDES}` verbatim
    # (sys/modules/iwlwifi/Makefile:79, rtw88:87, rtw89:82).
    (("compat/linuxkpi/", "contrib/dev/athk/", "contrib/dev/iwlwifi/",
      "contrib/dev/rtw88/", "contrib/dev/rtw89/", "contrib/dev/mediatek/",
      "contrib/dev/broadcom/", "contrib/dev/mt76/", "dev/mlx5/", "ofed/"),
     ("-I{S}/sys/compat/linuxkpi/common/include",
      "-I{S}/sys/compat/linuxkpi/dummy/include",
      "-include", "{S}/sys/compat/linuxkpi/common/include/linux/kconfig.h")),
    # sys/conf/kern.pre.mk:172-202 (CDDL_CFLAGS, common to dtrace and
    # zfs) plus :205-208 (ZFS_CFLAGS). Reproduced rather than guessed,
    # because the deciding macro is __KERNEL__ -- Linux's spelling, which
    # the FreeBSD kernel does NOT otherwise define. Without it
    # zfs_context.h:44 takes its USERLAND branch and asks for <unistd.h>,
    # which is why -D_KERNEL alone got 9 of 72 files under sys/cddl.
    (("contrib/openzfs/", "cddl/"),
     ("-DFREEBSD_NAMECACHE", "-D_SYS_VMEM_H_", "-D__KERNEL", "-D__KERNEL__",
      "-D_SYS_CONDVAR_H_", "-DBUILDING_ZFS", "-DHAVE_UIO_ZEROCOPY",
      "-include", "{S}/sys/modules/zfs/static_ccompile.h",
      "-include", "{S}/sys/contrib/openzfs/include/os/freebsd/spl/sys/ccompile.h",
      "-I{S}/sys/contrib/openzfs/include",
      "-I{S}/sys/contrib/openzfs/include/os/freebsd",
      "-I{S}/sys/contrib/openzfs/include/os/freebsd/spl",
      "-I{S}/sys/contrib/openzfs/include/os/freebsd/zfs",
      "-I{S}/sys/modules/zfs",
      "-I{S}/sys/contrib/openzfs/module/icp/include",
      "-I{S}/sys/contrib/openzfs/module/zstd/include",
      "-I{S}/sys/cddl/contrib/opensolaris/uts/common",
      "-I{S}/sys/cddl/compat/opensolaris")),
    # sys/conf/files:682 gives this to the in-kernel libsodium files, but
    # only to the ones the kernel builds; the rest of the vendor tree is
    # the same code and needs the same headers to be read at all.
    (("contrib/libsodium/", "crypto/libsodium/"),
     ("-I{S}/sys/contrib/libsodium/src/libsodium/include",
      "-I{S}/sys/contrib/libsodium/src/libsodium/include/sodium",
      "-I{S}/sys/crypto/libsodium")),
)


def _module_flags(rel_sys: str) -> list[str]:
    out = []
    for prefixes, flags in MODULE_INCLUDES:
        if rel_sys.startswith(prefixes):
            out += [f.format(S=SRC) for f in flags]
    # Every linuxkpi driver Makefile names itself, because the Linux code
    # it carries prints KBUILD_MODNAME in its own error messages
    # (sys/modules/iwlwifi/Makefile:97-98, rtw88:93-94, and so on for
    # each). The module is the directory under contrib/dev, so the name
    # is derivable rather than another table to keep in sync.
    if rel_sys.startswith("contrib/dev/"):
        mod = rel_sys.split("/")[2]
        out += [f'-DKBUILD_MODNAME="{mod}"', "-DLINUXKPI_VERSION=70000"]
    return out


def _subsystem_dirs(rel: str) -> list[str]:
    """sys/amd64/vmm/io/ppt.c -> sys/amd64/vmm/io, sys/amd64/vmm, sys/amd64.

    Nearest first, so a header shadowed by a closer one resolves the way
    the kernel build resolves it.
    """
    parts = rel.split("/")[:-1]          # drop the filename
    out = []
    while len(parts) > 1:                # stop before "sys" itself
        out.append((SRC / "/".join(parts)).as_posix())
        parts.pop()
    return out


def include_flags(src: Path, arch: str = "amd64", cc: str = "clang") -> list[str]:
    """-nostdinc plus everything that source needs, in build order."""
    try:
        rel = src.relative_to(SRC).as_posix() if src.is_absolute() else str(src)
    except ValueError:
        # Outside the tree - a probe or a scratch file. It gets the
        # compiler's own headers and nothing of FreeBSD's.
        return []
    arch = arch_of(rel, arch)

    # This is FreeBSD source, compiled on Linux, and the preprocessor was
    # answering Linux. clang and goto-cc both take their platform macros
    # from the host triple, so every translation unit in this sweep saw
    #
    #     __linux__ 1   __gnu_linux__ 1   __FreeBSD__ undefined
    #
    # 607 files under sys/, lib/libc and lib/msun key on one of those two
    # names - libsodium, ck, ACPICA, linuxkpi, zfs, and the libc bits that
    # ask which BSD they are on. Every `#ifdef __FreeBSD__` took its false
    # branch and every `#ifdef __linux__` its true one, so the checked code
    # was, in those files, not the code that ships.
    #
    # The -U/-D pair does the identity by hand, because goto-cc has no
    # --target to carry it (see TRIPLE / GOTO_ILP32 above) and the
    # identity is what those 607 files key on. __FreeBSD__ = 15 matches
    # the tree (__FreeBSD_version 1500000 in sys/sys/param.h), and it is
    # the same value clang's own freebsd15.0 triple predefines, so the
    # two agree rather than fight.
    flags = ["-nostdinc",
             *target_flags(arch, cc),
             "-U__linux__", "-U__gnu_linux__", "-D__FreeBSD__=15",
             f"-I{machine_shim(arch)}"]

    # The kernel is a different header universe from userland: no
    # include/, -D_KERNEL, and <sys/foo.h> resolving inside sys/. Mixing
    # the two puts userland's <stdio.h> in a kernel translation unit,
    # which is how you get a compile that succeeds and means nothing.
    if rel.startswith("sys/"):
        # sys/sys/kpilite.h:31 does `#include "offset.inc"`, which
        # genassym generates during buildkernel from sys/kern/genoffset.c
        # and which does not exist in a source tree. Its guard is
        #
        #   #if !defined(GENOFFSET) && (!defined(KLD_MODULE) || ...)
        #
        # so -DGENOFFSET skips it - and that is the honest spelling of the
        # situation here, where the offsets genuinely have not been
        # generated. -DKLD_MODULE reaches the same 85 of 95 but says
        # something false about how the code is being built.
        #
        # The cost is that sched_pin_lite() and sched_unpin_lite() are not
        # declared, so a file calling them still fails - recorded, not
        # papered over. 21 of 95 -> 85 of 95.
        # CBMC's C front end cannot parse C23 attribute syntax. 225 kernel
        # translation units failed on
        #
        #   sys/sys/systm.h:313:1: error: syntax error before '['
        #
        # which is __nodiscard, defined at cdefs.h:298 as [[nodiscard]].
        # cdefs.h:315 already defines it EMPTY on a compiler that does not
        # support it, so an empty definition is a configuration the header
        # itself provides rather than something invented here - and an
        # attribute that only affects diagnostics changes nothing a model
        # checker looks at.
        # ...and -D__nodiscard= does not work, because cdefs.h:301
        # REDEFINES it. The guard is
        #
        #   #elif defined(__STDC_VERSION__) && defined(__has_c_attribute)
        #   #if __has_c_attribute(__nodiscard__)
        #   #define __nodiscard [[__nodiscard__]]
        #
        # and CBMC's PREPROCESSOR answers __has_c_attribute yes while its
        # PARSER cannot read the result. Answering 0 sends cdefs.h down its
        # own `#else #define __nodiscard` branch - the empty definition the
        # header already provides for a compiler without the feature, which
        # is exactly true of this one.
        # What the tree says THIS file needs, and it goes first. Order
        # is not cosmetic here: sys/conf/kern.pre.mk:172-202 puts the
        # ZFS spl include dir ahead of -I$S and passes -D_SYS_CONDVAR_H_,
        # so that <sys/condvar.h> resolves to openzfs's and FreeBSD's is
        # suppressed. Append these after -I sys and the guard silences
        # the real header while nothing supplies the replacement --
        # `struct cv p_pwait` in sys/sys/proc.h:775 then has incomplete
        # type, which is a compile error invented entirely by flag order.
        rel_sys = rel[len("sys/"):]
        flags += list(conf_file_includes(arch).get(rel_sys, ()))
        flags += _module_flags(rel_sys)
        flags += ["-D_KERNEL", "-DGENOFFSET",
                  "-D__has_c_attribute(x)=0",
                  # The kernel build force-includes this into every
                  # translation unit (sys/conf/kern.pre.mk), and it is
                  # where config(8) puts the options that are not tied to
                  # one subsystem - INET's neighbours, INVARIANTS, SMP.
                  "-include", "opt_global.h",
                  f"-I{opt_shim(arch)}",
                  # device_if.h and friends: generated, not shipped.
                  f"-I{iface_shim()}",
                  f"-I{(SRC / rel).parent}",
                  # Every directory between sys/ and the file. A kernel
                  # module Makefile adds its own subsystem root - vmm's
                  # does `-I${SRCTOP}/sys/amd64/vmm` - and without it
                  # sys/amd64/vmm/io/ppt.c cannot find vmm_lapic.h, which
                  # is its sibling one level up, nor `io/iommu.h`, which is
                  # named relative to that root rather than to the file.
                  *[f"-I{d}" for d in _subsystem_dirs(rel)],
                  f"-I{SRC}/sys",
                  f"-I{SRC}/sys/contrib/ck/include",
                  f"-I{SRC}/sys/contrib/libnv",
                  # The opensolaris/dtrace/ZFS include set, which the
                  # module Makefiles add and nothing else does.
                  # sys/modules/dtrace/dtrace/Makefile:47-50 is the list;
                  # without it 18 files fail on <sys/dtrace.h> and 12 more
                  # on <sys/dtrace_impl.h>, in sys/cddl alone.
                  f"-I{SRC}/sys/cddl/compat/opensolaris",
                  f"-I{SRC}/sys/cddl/contrib/opensolaris/uts/common",
                  f"-I{SRC}/sys/cddl/contrib/opensolaris/uts/common/fs/zfs",
                  f"-I{SRC}/sys/cddl/contrib/opensolaris/common/zfs",
                  f"-I{SRC}/sys/cddl/contrib/opensolaris/uts/intel"]
        rd = resource_dir(cc)
        if rd:
            flags.append(f"-I{rd}")
        return flags

    # The source's own directory first: many libc and msun sources include a
    # private header sitting beside them.
    flags.append(f"-I{(SRC / rel).parent}")

    if rel.startswith("lib/libc"):
        flags += [f"-I{SRC}/lib/libc/include",
                  f"-I{SRC}/lib/libc/{LIBC_ARCH.get(arch, 'amd64')}",
                  f"-I{SRC}/lib/libc/resolv",
                  f"-I{SRC}/lib/libc/softfloat",
                  f"-I{SRC}/lib/libc/softfloat/templates",
                  f"-I{SRC}/lib/libc/{LIBC_ARCH.get(arch, 'amd64')}/softfloat",
                  f"-I{SRC}/lib/libc/gdtoa",
                  f"-I{SRC}/contrib/gdtoa",
                  f"-I{SRC}/lib/libc/locale",
                  f"-I{SRC}/lib/libc/stdio",
                  f"-I{SRC}/lib/libc/net",
                  f"-I{SRC}/lib/msun/src"]
    if rel.startswith("lib/msun"):
        # _fpmath.h is libc's, per architecture - lib/msun/Makefile adds
        # -I${LIBC_SRCTOP}/${LIBC_ARCH} for exactly this.
        flags += [f"-I{SRC}/lib/msun/src", f"-I{SRC}/lib/msun/ld80",
                  f"-I{SRC}/lib/msun/ld128", f"-I{SRC}/lib/msun/x86",
                  f"-I{SRC}/lib/libc/include",
                  f"-I{SRC}/lib/libc/{LIBC_ARCH.get(arch, 'amd64')}"]
    if rel.startswith("lib/libmd"):
        flags.append(f"-I{SRC}/lib/libmd")
    if "/tests/" in rel:
        # A test program is not the library, but it is C in this tree and
        # it exercises the library's edge cases, which is where the
        # defects are. All 179 of them wanted <atf-c.h>, and it is in the
        # tree at contrib/atf - share/mk/atf.test.mk is what adds it to a
        # real build.
        flags.append(f"-I{SRC}/contrib/atf")

    flags += [f"-I{SRC}/lib/libsys", f"-I{SRC}/include", f"-I{SRC}/sys"]
    rd = resource_dir(cc)
    if rd:
        flags.append(f"-I{rd}")
    return flags


# Not everything under sys/ is a kernel translation unit, and compiling
# the ones that are not produces an error that says nothing: they are
# userland C, they want <stdio.h> and <unistd.h>, and -D_KERNEL means
# they will never find them. They are not failures to fix - they are
# files no FreeBSD kernel builds, and this is the list with the reason.
NOT_KERNEL = (
    # OpenZFS ships its whole upstream repo. sys/modules/zfs/Makefile
    # takes .PATH only under module/, and sys/conf/files.* names nothing
    # else; cmd/, tests/, udev/, etc/, lib/ and contrib/ are the Linux
    # distribution's userland and are built, if at all, under cddl/.
    "contrib/openzfs/cmd/", "contrib/openzfs/tests/",
    "contrib/openzfs/udev/", "contrib/openzfs/etc/",
    "contrib/openzfs/lib/", "contrib/openzfs/contrib/",
    "contrib/openzfs/scripts/",
    # ...and its Linux kernel port, which FreeBSD replaces wholesale
    # with module/os/freebsd/.
    "contrib/openzfs/module/os/linux/",
    # ACPICA likewise: common/, compiler/ and tools/ are iasl, acpidump
    # and acpiexec, built from usr.sbin/acpi. Only the interpreter and
    # the dispatcher under contrib/dev/acpica/components/ is kernel.
    "contrib/dev/acpica/common/", "contrib/dev/acpica/compiler/",
    "contrib/dev/acpica/tools/",
    "contrib/dev/acpica/os_specific/service_layers/oswinxf.c",
    "contrib/dev/acpica/os_specific/service_layers/osunix",
    "contrib/dev/acpica/os_specific/service_layers/oswin",
    "contrib/dev/acpica/os_specific/service_layers/osl",
)


def is_kernel_tu(rel: str) -> bool:
    """False for the userland C that happens to live under sys/."""
    if not rel.startswith("sys/"):
        return True
    return not rel[len("sys/"):].startswith(NOT_KERNEL)


def lang_flags(src: Path) -> list[str]:
    return (["-xc++", "-std=c++23", "-fno-exceptions", "-fno-rtti"]
            if src.suffix == ".cpp" else ["-xc", "-std=c17"])


if __name__ == "__main__":
    import sys
    for a in sys.argv[1:]:
        p = Path(a)
        if not p.is_absolute():
            p = SRC / a
        print(" ".join(lang_flags(p) + include_flags(p)))
