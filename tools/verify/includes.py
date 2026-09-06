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


def arch_of(rel: str, default: str = "amd64") -> str:
    """A source under lib/libc/<arch>/ or lib/msun/<arch>/ is that arch.

    Verifying lib/libc/aarch64/gen/getcontextx.c against amd64's
    <machine/*.h> is not a check of anything; it is a different program.
    """
    parts = rel.split("/")
    if len(parts) > 2 and parts[0] == "lib" and parts[1] in ("libc", "msun"):
        cand = ARCH_DIR.get(parts[2])
        if cand:
            return cand
    return default


def include_flags(src: Path, arch: str = "amd64", cc: str = "clang") -> list[str]:
    """-nostdinc plus everything that source needs, in build order."""
    rel = src.relative_to(SRC).as_posix() if src.is_absolute() else str(src)
    arch = arch_of(rel, arch)
    flags = ["-nostdinc", f"-I{machine_shim(arch)}"]

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
