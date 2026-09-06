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
def opt_shim() -> str:
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
    for n in names:
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
    # -U/-D rather than --target=: goto-cc is a gcc driver and does not
    # accept --target, and the identity is the whole of what needs to
    # change. __FreeBSD__ = 15 matches the tree (__FreeBSD_version 1500000
    # in sys/sys/param.h).
    flags = ["-nostdinc",
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
        flags += ["-D_KERNEL", "-DGENOFFSET",
                  "-D__has_c_attribute(x)=0",
                  f"-I{opt_shim()}",
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
                  f"-I{SRC}/sys/cddl/compat/opensolaris"]
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

    flags += [f"-I{SRC}/lib/libsys", f"-I{SRC}/include", f"-I{SRC}/sys"]
    rd = resource_dir(cc)
    if rd:
        flags.append(f"-I{rd}")
    return flags


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
