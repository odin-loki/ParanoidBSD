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

import collections
import functools
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import userland_names

USERLAND_TOP = ("lib", "libexec", "bin", "sbin", "usr.bin",
                "usr.sbin", "libexec", "games", "secure", "cddl")

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"
SYS = SRC / "sys"

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
            "riscv": "riscv64", "powerpcspe": "powerpc64",
            # lib/msun/Makefile:13 - ARCH_SUBDIR is
            # ${MACHINE_CPUARCH:S/i386/i387/}, so lib/msun/i387 IS i386.
            # Called amd64, i387/fenv.c fails on `__SSE_YES', which
            # lib/msun/x86/fenv.h declares.
            "i387": "i386"}


# The KERNEL's per-architecture directory names, which are a third
# spelling again: sys/arm64 where libc says aarch64 and the ARCH table's
# key is aarch64. sys/<this> -> the ARCH key to build the shim from.
SYS_ARCH = {"amd64": "amd64", "arm64": "aarch64", "arm": "armv7",
            "i386": "i386", "powerpc": "powerpc64", "riscv": "riscv64",
            # ...and sys/cddl uses the TOOLCHAIN's spelling for the same
            # directories - sys/cddl/dev/dtrace/aarch64, .../riscv64,
            # .../powerpc64 - so both spellings map to the same shim.
            "aarch64": "aarch64", "riscv64": "riscv64",
            "powerpc64": "powerpc64"}


# A libc subdirectory's Makefile.inc is where its -D flags live, and
# without them the directory does not compile: lib/libc/posix1e is
# -D_ACL_PRIVATE, without which <sys/acl.h> makes acl_t a `void *' and
# every acl->ats_brand is "member reference base type void". Twelve
# translation units of the POSIX.1e and NFSv4 ACL layer, on one -D from
# a three-line Makefile.
#
# lib/libc/db is -D__DBINTERFACE_PRIVATE, lib/libc/regex is
# -DPOSIX_MISTAKE, lib/libc/rpc is -DBROKEN_DES -DPORTMAP -DDES_BUILTIN,
# lib/libc/net is -DINET6. Reading them out of the Makefile is the only
# form of this that does not go stale: a table here is a second place
# for the answer to be wrong in, and the softfloat and csu cases already
# showed what that costs.
CFLAGS_LINE = re.compile(r"^CFLAGS(?:\.(?P<file>\S+))?\s*[+?]?=\s*(?P<rest>.*)$")
MAKE_VAR = re.compile(r"^(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*[+?]?=\s*(?P<val>.*)$")
DEFINE = re.compile(r"-D[A-Za-z_]\w*(?:=\S+)?")
INCLUDE = re.compile(r"-I(\S+)")
VAR_REF = re.compile(r"\$\{(\.?[A-Za-z_][A-Za-z0-9_.]*)\}")


def _expand(text: str, vars: dict[str, str]) -> str:
    """${FOO} against the Makefile's own assignments, three deep.

    A reference that does not resolve is left alone, and the caller
    drops any flag that still contains one. Guessing at ${MK_SOMETHING}
    would be inventing a build rather than reading it.
    """
    for _ in range(3):
        new = VAR_REF.sub(lambda m: vars.get(m.group(1), m.group(0)), text)
        if new == text:
            break
        text = new
    return text


@functools.lru_cache(maxsize=None)
def makefile_flags(d: str, arch: str, base: str) -> tuple[tuple[str, ...],
                                                          tuple[str, ...]]:
    """(-D, -I) a directory's own Makefile puts on CFLAGS, expanded.

    The build system already knows every answer this sweep has been
    guessing at, and each guess has been wrong in its own way:
    lib/libc/posix1e is one -D_ACL_PRIVATE, without which <sys/acl.h>
    makes acl_t a `void *' and all twelve translation units of the
    POSIX.1e and NFSv4 ACL layer die on "member reference base type
    void". lib/libc/gen wants -I${SRCTOP}/contrib/libc-vis for <vis.h>
    and ${RTLD_HDRS} for dlfcn.c. libexec/bootpd/bootpgw wants
    -I${SRCDIR}, which that Makefile defines two lines above.

    So read them. Unconditional lines only - a CFLAGS inside .if/.endif
    is a build-time choice this sweep has not made, and applying
    lib/libc/softfloat's -DFLOAT128 (under `.if defined(SOFTFLOAT_128)')
    would compile a quad-precision softfloat no libc in this tree
    builds. Per-file CFLAGS.<name> lines count only for that file.
    """
    dpath = Path(d)
    defines: list[str] = []
    includes: list[str] = []
    vars = {
        "SRCTOP": str(SRC),
        ".CURDIR": d,
        "LIBC_SRCTOP": str(SRC / "lib/libc"),
        "LIBC_ARCH": LIBC_ARCH.get(arch, "amd64"),
        "MACHINE_CPUARCH": LIBC_ARCH.get(arch, "amd64"),
        "MACHINE_ARCH": arch,
        "RTLD_HDRS": f"-I{SRC}/libexec/rtld-elf",
    }
    for name in ("Makefile.inc", "Makefile"):
        mk = dpath / name
        if not mk.is_file():
            continue
        depth = 0
        for line in mk.read_text(errors="replace").replace("\\\n",
                                                           " ").splitlines():
            st = line.strip()
            if st.startswith((".if", ".for")):
                depth += 1
                continue
            if st.startswith((".endif", ".endfor")):
                depth = max(0, depth - 1)
                continue
            if depth or not st:
                continue
            if st.startswith(".PATH:"):
                # .PATH names the directories make finds the SOURCES in,
                # and a source's private header sits beside it there:
                # lib/libc/stdtime takes .PATH on contrib/tzcode, where
                # private.h lives, and lib/libc/gen takes it on
                # contrib/libc-vis for vis.h. The sweep compiles each
                # file where it lies rather than where make would put
                # it, so those directories have to be on the include
                # path or the header beside the source is unreachable.
                for d2 in _expand(st[len(".PATH:"):], vars).split():
                    if "${" not in d2 and Path(d2).is_dir():
                        includes.append(f"-I{d2}")
                continue
            m = CFLAGS_LINE.match(line)
            if m:
                if m.group("file") and m.group("file") != base:
                    continue
                rest = _expand(m.group("rest"), vars)
                defines.extend(f for f in DEFINE.findall(rest)
                               if "${" not in f)
                for inc in INCLUDE.findall(rest):
                    if "${" not in inc and Path(inc).is_dir():
                        includes.append(f"-I{inc}")
                continue
            v = MAKE_VAR.match(line)
            if v and v.group("name") not in ("CFLAGS", "SRCS", "MLINKS",
                                             "MAN", "SYM_MAPS"):
                vars.setdefault(v.group("name"),
                                _expand(v.group("val").strip(), vars))
    return tuple(defines), tuple(includes)


# The kernel says, per driver, what a driver needs - in two places, both
# authoritative and neither previously read:
#
#   sys/conf/files*        compile-with "${NORMAL_C} -I$S/contrib/ck/include"
#   sys/modules/*/Makefile .PATH on the source directory, plus its CFLAGS
#
# sys/dev alone had 1,510 translation units of 5,695 come back ERROR, and
# whole drivers are in that number for want of one -I: 84 for Intel QAT
# (<cpa.h>, <qat_freebsd.h>, <adf_accel_devices.h>, all under
# sys/dev/qat/, all named by sys/modules/qat/Makefile), 31 for the DPAA
# ethernet, twenty-odd for anything using linuxkpi. The dtrace include
# set below was the first instance of this and was solved by copying one
# module Makefile's four lines into this file; that does not scale to
# 115 module Makefiles, and a copy goes stale where a read does not.
#
# The quotes on a compile-with are optional. sys/conf/files:644-664 is twenty-one lines of
#
#   contrib/zstd/lib/common/error_private.c  optional zstdio \
#       compile-with ${ZSTD_C}
#
# and ZSTD_C (sys/conf/kern.pre.mk:160) carries the three -I that make
# zstd's own headers findable. Requiring the quotes cost every one of
# them: the whole in-kernel zstd, which is zstdio(9) and the ZFS
# compressor, came back "missing header: stdlib.h" - it had not found
# <zstd_deps.h>, the shim that maps those onto the kernel.
FILES_COMPILE = re.compile(
    r'^(?P<src>\S+\.c)\s+\S+.*?compile-with\s+'
    r'(?:"(?P<dq>[^"]*)"|\'(?P<sq>[^\']*)\'|(?P<bare>\S+))')
DASH_INCLUDE = re.compile(r"-include\s+(\S+)")


def _kernel_dirs(text: str, vars: dict[str, str]) -> list[str]:
    """The -I and -include in a compile-with or a module's CFLAGS.

    Expand the whole line before matching, not each token after: half
    these lines are a bare ${LINUXKPI_INCLUDES}, which is three flags
    and matches neither regex until it has been substituted. That is
    what left every linuxkpi-using driver - qat among them - still
    failing on <linux/types.h> after its own -I had been found.
    """
    # $S is make's own abbreviation for the kernel source root and is
    # bare, not ${S}, so _expand does not see it - and when it arrives
    # inside an expanded variable's VALUE (DPAA_COMPILE_CMD is twenty-one
    # -I$S/...) replacing it before expansion is too early.
    text = _expand(text, vars).replace("$S", str(SYS))
    out = []
    # The -D as well as the -I. An -I makes a header findable; a -D
    # decides what is in it, and for the vendored Linux drivers that is
    # the whole difference: without -DCONFIG_IWLMLD=1 and -DCONFIG_PM,
    # iwlwifi's headers declare a different `struct iwl_mld' than its
    # sources use, and the file does not compile at all.
    for tok in DEFINE.findall(text):
        if "$" not in tok:
            out.append(tok)
    for tok in DASH_INCLUDE.findall(text):
        if "$" not in tok and Path(tok).is_file():
            out.append(f"-include{tok}")
    for tok in INCLUDE.findall(DASH_INCLUDE.sub("", text)):
        # `-I.' is the kernel build directory, not the analyser's cwd,
        # and there is nothing at the latter this sweep should read.
        if "$" not in tok and tok.startswith("/") and Path(tok).is_dir():
            out.append(f"-I{tok}")
    return out


@functools.lru_cache(maxsize=None)
def kern_pre_vars() -> dict[str, str]:
    """The unconditional assignments in sys/conf/kern.pre.mk.

    This file was previously represented here by a hand-written table of
    three flags for LINUXKPI_INCLUDES and three more for OFEDINCLUDES,
    copied out of it. The table was right about those two and knew
    nothing else, so `compile-with "${LINUXKPI_C}"' - which is
    ${NORMAL_C} ${LINUXKPI_INCLUDES}, and which sys/powerpc/conf/dpaa
    builds twenty-one -I on top of - expanded to nothing, and five
    built DPAA translation units failed on <linux/math64.h> that this
    tree ships. A copy is a second place for the answer to be wrong in;
    read the file.

    Only depth-0 assignments are taken: the conditional ones are per
    compiler and per architecture and this sweep is neither.
    """
    out: dict[str, str] = {}
    # Both of them. kern.pre.mk is what a kernel build reads and
    # kmod.mk is what every module Makefile reaches through its closing
    # `.include <bsd.kmod.mk>' - and OPENZFS_CFLAGS, the twelve -I that
    # put opensolaris's <sys/types.h> ahead of FreeBSD's, is defined at
    # kmod.mk:576 and nowhere else. Reading only the first left
    # sys/modules/dtrace/sdt/Makefile with `CFLAGS+= ${OPENZFS_CFLAGS}'
    # expanding to nothing, which then failed the `if not flags' test
    # below and threw away the module's .PATH as well.
    for mk in (SYS / "conf" / "kern.pre.mk", SYS / "conf" / "kmod.mk"):
        if not mk.is_file():
            continue
        depth = 0
        for line in mk.read_text(errors="replace").replace(
                "\\\n", " ").splitlines():
            st = line.strip()
            if st.startswith((".if", ".for")):
                depth += 1
                continue
            if st.startswith((".endif", ".endfor")):
                depth = max(0, depth - 1)
                continue
            if depth or not st or st.startswith("#"):
                continue
            m = MAKE_VAR.match(st)
            if m:
                out.setdefault(m.group("name"), m.group("val").strip())
    return out


# `.if' in a module Makefile is not always a question the tool cannot
# answer. sys/modules/iwlwifi/Makefile sets IWLWIFI_CONFIG_PM=1 at the
# top and then
#
#   .if defined(IWLWIFI_CONFIG_PM) && ${IWLWIFI_CONFIG_PM} > 0
#   SRCS+= mvm/d3.c mld/d3.c lkpi_iwlwifi_pm.c
#   CFLAGS+= -DCONFIG_PM
#   CFLAGS+= -DCONFIG_PM_SLEEP
#   .endif
#
# which is a block the build always takes, from a variable set five
# lines up in the same file. Skipping every conditional lost those two
# -D. Three-valued on purpose: undecidable skips, exactly as before,
# because guessing at ${MK_SOMETHING} or ${KERN_OPTS:M...} would be
# inventing a build rather than reading one.
MK_DEFINED = re.compile(r"defined\(([A-Za-z_]\w*)\)")
MK_CMP = re.compile(r"\$\{([A-Za-z_]\w*)\}\s*(==|!=|>=|<=|>|<)\s*"
                    r'"?([A-Za-z0-9_.]*)"?')


def _mk_cond(expr: str, vars: dict[str, str]) -> bool | None:
    """Truth of a module Makefile `.if', or None when not decidable."""
    parts = re.split(r"\s*(&&|\|\|)\s*", expr.strip())
    if not parts or not parts[0]:
        return None
    vals: list[bool | None] = []
    ops: list[str] = []
    for i, tok in enumerate(parts):
        if i % 2:
            ops.append(tok)
            continue
        tok = tok.strip()
        m = MK_DEFINED.fullmatch(tok)
        if m:
            vals.append(m.group(1) in vars)
            continue
        m = MK_CMP.fullmatch(tok)
        if not m:
            return None
        name, op, rhs = m.groups()
        if name not in vars:
            vals.append(None)
            continue
        lhs = vars[name].strip()
        try:
            a: object = int(lhs)
            b: object = int(rhs)
        except ValueError:
            if op not in ("==", "!="):
                vals.append(None)
                continue
            a, b = lhs, rhs
        vals.append({"==": a == b, "!=": a != b,
                     ">": a > b, "<": a < b,          # type: ignore[operator]
                     ">=": a >= b, "<=": a <= b}[op])  # type: ignore[operator]
    out = vals[0]
    for op, v in zip(ops, vals[1:]):
        if op == "&&":
            out = False if False in (out, v) else \
                (None if None in (out, v) else True)
        else:
            out = True if True in (out, v) else \
                (None if None in (out, v) else False)
    return out


# make's own idea of the machine, in the three spellings a module
# Makefile uses. sys/modules/dtrace/dtrace/Makefile is
#
#   ARCHDIR=	${MACHINE_CPUARCH}
#   .PATH: ${SYSDIR}/cddl/dev/dtrace/${ARCHDIR}
#   .if ${MACHINE_CPUARCH} == "amd64" || ${MACHINE_CPUARCH} == "i386"
#   CFLAGS+= -I${SYSDIR}/cddl/contrib/opensolaris/uts/intel ...
#
# and with none of the three defined that .PATH resolved to nothing and
# that .if was undecidable, so DTrace, FBT, kinst, SDT and the CTF
# reader all failed on their own arch-private headers - regset.h,
# fbt_isa.h, kinst_isa.h, dis_tables.h. This is why the index is per
# architecture rather than one table for the tree.
MACHINE_OF = {
    "amd64":     ("amd64", "amd64", "amd64"),
    "i386":      ("i386", "i386", "i386"),
    "aarch64":   ("aarch64", "aarch64", "arm64"),
    "armv7":     ("arm", "armv7", "arm"),
    "powerpc64": ("powerpc", "powerpc64", "powerpc"),
    "riscv64":   ("riscv", "riscv64", "riscv"),
}


@functools.lru_cache(maxsize=None)
def kernel_flag_index(arch: str = "amd64"
                      ) -> tuple[dict[str, tuple[str, ...]],
                                 dict[str, tuple[str, ...]]]:
    """(by source path, by directory) the flags a kernel build adds."""
    by_file: dict[str, list[str]] = {}
    by_src: dict[str, list[str]] = {}
    by_dir: dict[str, list[str]] = {}
    base = dict(kern_pre_vars())
    # These three are make's, not kern.pre.mk's, and are the roots the
    # rest resolve against: they win over anything read out of a file.
    base.update({"SRCTOP": str(SRC), "SYSDIR": str(SYS), "S": str(SYS)})
    cpuarch, marcH, mach = MACHINE_OF.get(arch, MACHINE_OF["amd64"])
    base.update({"MACHINE_CPUARCH": cpuarch, "MACHINE_ARCH": marcH,
                 "MACHINE": mach})

    # A compile-with can name a variable rather than spell the flags out:
    #
    #   contrib/ncsw/etc/error.c  optional dpaa \
    #       no-depend compile-with "${DPAA_COMPILE_CMD}"
    #
    # and DPAA_COMPILE_CMD is a `makeoptions' in
    # sys/powerpc/conf/dpaa/config.dpaa, twenty-one -I long. Without it
    # the whole DPAA ethernet - fifty-one translation units, the
    # networking on every QorIQ board - fails on <std_ext.h>. Collect
    # every makeoptions in every architecture's conf/ before reading the
    # files* that use them.
    MAKEOPT = re.compile(r'^\s*makeoptions\s+([A-Za-z_]\w*)\s*[+?]?=\s*(.*)$')
    for cf in sorted(SYS.glob("*/conf/**/*")):
        if not cf.is_file() or cf.suffix in (".c", ".h", ".m"):
            continue
        try:
            text = cf.read_text(errors="replace").replace("\\\n", " ")
        except (OSError, UnicodeDecodeError):
            continue
        for line in text.splitlines():
            m = MAKEOPT.match(line)
            if not m:
                continue
            val = m.group(2).strip()
            if val.startswith('"'):
                end = val.find('"', 1)
                val = val[1:end] if end > 0 else val[1:]
            else:
                # a bare value ends at make's comment character, and
                # sixteen configs write `makeoptions WITH_CTF=1 # ...'
                val = val.split("#", 1)[0].strip()
            base.setdefault(m.group(1), val)

    files = sorted((SYS / "conf").glob("files*")) + \
        sorted(f for f in SYS.rglob("files.*")
               if f.is_file() and f.parent != SYS / "conf")
    for mk in files:
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_COMPILE.match(line)
            if not m:
                continue
            cmd = m.group("dq") or m.group("sq") or m.group("bare") or ""
            fl = _kernel_dirs(cmd.replace("$S", str(SYS)), base)
            if fl:
                by_file.setdefault("sys/" + m.group("src"), []).extend(fl)

    for mk in sorted((SYS / "modules").rglob("Makefile")):
        # bmake pulls ${.CURDIR}/../Makefile.inc in from bsd.init.mk,
        # which every one of these Makefiles reaches through its
        # `.include <kmod.opts.mk>' on line 3. sys/modules/mt76/*/Makefile
        # opens `.PATH: ${COMMONDIR}' and COMMONDIR is defined in
        # sys/modules/mt76/Makefile.inc, one directory up - so the
        # .PATH resolved to nothing and every mt76 driver, 135
        # translation units, found none of its own headers. Walk up to
        # sys/modules collecting them; nested submodules inherit each
        # level, which is exactly what bmake does.
        incs = []
        d = mk.parent.parent
        while d != SYS / "modules" and d != d.parent:
            inc = d / "Makefile.inc"
            if inc.is_file():
                incs.append(inc)
            d = d.parent
        if (SYS / "modules" / "Makefile.inc").is_file():
            incs.append(SYS / "modules" / "Makefile.inc")

        text = mk.read_text(errors="replace").replace("\\\n", " ")
        vars = dict(base)
        vars[".CURDIR"] = str(mk.parent)
        # Two passes. The Makefile's own assignments come first (its
        # `MT76_DRIVER_NAME= mt7615' is line 1 and the .inc's DEVDIR
        # expands it), then the .inc chain nearest-first; then the whole
        # lot is re-read for .PATH and CFLAGS with the table complete,
        # because a .PATH can name a variable defined in a file read
        # after it and an .if can test one.
        for src in [text] + [i.read_text(errors="replace").replace("\\\n", " ")
                             for i in incs]:
            # A block this pass can prove FALSE is skipped; one it
            # cannot decide is taken. That asymmetry is the assumption
            # the whole sweep already runs on - a GENERIC-like kernel of
            # this architecture - and it is what supplies
            # `IWLWIFI_CONFIG_ACPI= 1' from inside
            # `.if ${KERN_OPTS:MDEV_ACPI}', which every amd64 and arm64
            # config satisfies. Taking a definitely-dead assignment
            # instead would let a variable nobody sets decide a later
            # condition, which is how -DCONFIG_ACPI first arrived here:
            # by accident.
            dead = 0
            for line in src.splitlines():
                st = line.strip()
                if st.startswith((".if", ".for")):
                    if dead:
                        dead += 1
                    elif st.startswith(".ifdef"):
                        dead = 1 if st.partition(" ")[2].strip() not in vars else 0
                    elif st.startswith(".ifndef"):
                        dead = 1 if st.partition(" ")[2].strip() in vars else 0
                    elif st.startswith(".if"):
                        dead = 1 if _mk_cond(st.partition(" ")[2],
                                             vars) is False else 0
                    continue
                if st.startswith((".endif", ".endfor")):
                    dead = max(0, dead - 1)
                    continue
                if st.startswith((".else", ".elif")):
                    # The complement of a block we skipped is live, and
                    # of one we took is dead. Undecidable stays taken.
                    dead = 0 if dead == 1 else dead
                    continue
                if dead:
                    continue
                v = MAKE_VAR.match(line)
                if v and v.group("name") not in ("CFLAGS", "SRCS"):
                    vars.setdefault(v.group("name"),
                                    _expand(v.group("val").strip(), vars))
        # A variable whose value named another that was not known yet.
        for k, v2 in list(vars.items()):
            if "${" in v2:
                vars[k] = _expand(v2, vars)

        paths: list[str] = []
        flags: list[str] = []
        srcs: list[str] = []
        # A stack of "is this block live?", so a nested .if inside a
        # skipped one stays skipped.
        live: list[bool] = []
        for line in "\n".join(
                [text] + [i.read_text(errors="replace").replace("\\\n", " ")
                          for i in incs]).splitlines():
            st = line.strip()
            if st.startswith(".if"):
                cond = None
                if all(live):
                    kw, _, rest = st.partition(" ")
                    if kw == ".ifdef":
                        cond = rest.strip() in vars
                    elif kw == ".ifndef":
                        cond = rest.strip() not in vars
                    elif kw == ".if":
                        cond = _mk_cond(rest, vars)
                live.append(cond is True)
                continue
            if st.startswith(".for"):
                live.append(False)
                continue
            if st.startswith((".else", ".elif")):
                # Never take an else: the .if it belongs to was either
                # taken (so this is dead) or undecidable (so this is).
                if live:
                    live[-1] = False
                continue
            if st.startswith((".endif", ".endfor")):
                if live:
                    live.pop()
                continue
            if not all(live) or not st:
                continue
            if st.startswith(".PATH:"):
                for d in _expand(st[len(".PATH:"):].replace("$S", str(SYS)),
                                 vars).split():
                    if "${" not in d and Path(d).is_dir():
                        paths.append(d)
                continue
            m = CFLAGS_LINE.match(line)
            if m and not m.group("file"):
                flags.extend(_kernel_dirs(
                    m.group("rest").replace("$S", str(SYS)), vars))
                continue
            m = MODULE_SRCS.match(line)
            if m:
                srcs.extend(x for x in _expand(m.group(1), vars).split()
                            if x.endswith(".c") and "${" not in x)
                continue
        if not flags:
            continue

        # The files this module actually names, resolved against its
        # .PATH set. A directory entry is too coarse on its own:
        # sys/modules/dtrace/dtnfscl takes .PATH on sys/fs/nfsclient and
        # builds ONE file from it, and tagging the directory gave every
        # other NFS client source ${OPENZFS_CFLAGS} - twelve -I that put
        # openzfs's SPL <sys/rwlock.h> ahead of FreeBSD's. Harmless
        # while those flags were appended last; the moment they moved in
        # front of -I$S, as sys/conf/kmod.mk:128 says they are,
        # nfs_clsubs.c and nfs_clkrpc.c went from clean to seventeen
        # errors on rw_assert and RA_WLOCKED. Found by compiling all 72
        # order-changed files both ways rather than reasoning about the
        # flag sets.
        for s in srcs:
            for base_dir in paths:
                f = (Path(base_dir) / s).resolve()
                if not f.is_file():
                    continue
                try:
                    by_src.setdefault(
                        f.relative_to(SRC.resolve()).as_posix(),
                        []).extend(flags)
                except ValueError:
                    pass
                break

        # A module that builds objects OUTSIDE SrcS is invisible to the
        # reading above, and the seven that do are the ones with
        # per-file instruction-set flags. sys/modules/blake2 is the
        # shape:
        #
        #   SRCS_IN += blake2b-avx.c
        #   OBJS    += ${SRCS_IN:S/.c/.o/g}
        #   .for src in ${SRCS_IN}
        #   ${src:S/.c/.o/}: ${src}
        #           ${CC} -c ${CFLAGS:N-nostdinc} ${CFLAGS.${src}} ...
        #
        # so no SRCS line names its ten SIMD implementations, and none
        # of them compiles without CFLAGS.blake2b-avx.c, which is
        # `-DSUFFIX=_avx -msse2 -mssse3 -msse4.1 -mavx'. Expanding
        # ${SRCS_IN:S/.c/.o/g} here would be a third implementation of
        # bmake's modifiers; ask bmake. Only where an OBJS line exists -
        # seven Makefiles in the tree - so the cost is seven bmake runs
        # and not a thousand.
        if re.search(r"^OBJS\s*\+?=", text, re.M):
            per, _, obj_rels = userland_names.ask_module(mk.parent, arch,
                                                        SRC)
            # A .for rule compiles with its OWN command line, and the -D
            # on it are not in any variable bmake will hand back.
            # blake2's is
            #
            #   ${CC} -c ${CFLAGS:N-nostdinc} ${CFLAGS.${src}} ${WERROR} \
            #       ${PROF} -D_MM_MALLOC_H_INCLUDED -Wno-unused-function
            #
            # and without that -D, clang's own <mm_malloc.h> - which
            # <immintrin.h> pulls in - calls malloc() and free() with no
            # <stdlib.h>, in a kernel translation unit. The rule bodies
            # are the tab-indented lines; take their literal -D and
            # nothing else.
            rules = [line for line in text.splitlines()
                     if line.startswith("\t")]
            rule_d = [w for line in rules for w in line.split()
                      if w.startswith("-D") and "$" not in w and len(w) > 2]
            # ...and `${CFLAGS:N-nostdinc}' is the rule saying it wants
            # the standard headers, which is the other half of the same
            # decision: -D_MM_MALLOC_H_INCLUDED does not silence clang's
            # own <mm_malloc.h> (its guard is __MM_MALLOC_H), so
            # <immintrin.h> reaches malloc() and free() and needs a real
            # <stdlib.h>. A marker flag the analyser strips later,
            # rather than a second flag list to keep in step.
            if any(":N-nostdinc" in line for line in rules):
                rule_d.append("-DPBSD_WANTS_STDINC")
            # ONLY the .for-rule sources. The SRCS the hand reading
            # above already covers, and adding them here again gave
            # sys/compat/linux the module's -DLOCORE, which the reading
            # above deliberately keeps in by_dir and out of by_src.
            # Twenty-six regressions in one sweep, all of them a header
            # told the file was assembly.
            for rel_src in sorted(obj_rels):
                own = per.get(rel_src, ())
                by_src.setdefault(rel_src, []).extend(
                    list(flags) + rule_d
                    + [f for f in own
                       if f.startswith(("-I", "-D", "-U", "-m"))])

        # ...and the directory, for the files under a .PATH that the
        # SRCS do not name. Dropping it where the SRCS resolved looked
        # tidier and cost six files their include set - the TX99 corner
        # of the ath HAL, brcmfmac's ring code, amd64's linux32
        # genassym - all siblings of files a module does build. What
        # the directory must NOT do is decide flag ORDER, which is why
        # include_flags keeps it at the end; see there.
        for d in paths:
            try:
                key = str(Path(d).relative_to(SRC))
            except ValueError:
                continue
            by_dir.setdefault(key, []).extend(flags)

    return ({k: tuple(dict.fromkeys(v)) for k, v in by_file.items()},
            {k: tuple(dict.fromkeys(v)) for k, v in by_src.items()},
            {k: tuple(dict.fromkeys(v)) for k, v in by_dir.items()})


# ...and the same trick for the architecture. sys/dev/cesa/cesa.c is a
# Marvell ARM crypto engine and nothing in its path says so; it is named
# by sys/arm/mv/files.arm7, and analysed as amd64 it fails on
# <machine/intr.h>. There are ~90 files.* under sys/ outside conf/ -
# every SoC and every platform has one - and between them they say which
# architecture each driver belongs to. 14 translation units failed on
# machine/intr.h alone, and the rest of that pile is the same thing:
# ext_resources, FDT regulators, DPAA, dbdma.
FILES_SRC = re.compile(r"^(\S+\.c)\s")
MODULE_SRCS = re.compile(r"^\s*SRCS(?:\.\w+)?\s*\+?=\s*(.*)$")


@functools.lru_cache(maxsize=None)
def files_arch_index() -> dict[str, str]:
    """source path under sys/ -> the architecture whose files.* names it."""
    out: dict[str, str] = {}
    for mk in SYS.rglob("files*"):
        if not mk.is_file() or mk.suffix in (".c", ".h"):
            continue
        parts = mk.relative_to(SYS).parts
        if parts[0] == "conf":
            arch = SYS_ARCH.get(mk.name.partition(".")[2])
        else:
            arch = SYS_ARCH.get(parts[0])
        if not arch:
            continue
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_SRC.match(line)
            if m:
                # First writer wins, so a driver named by two SoCs of the
                # same architecture is not fought over, and one named by
                # conf/files (no architecture) never reaches here at all.
                out.setdefault("sys/" + m.group(1), arch)
    return out


# `cpu' is not an option a kernel may or may not want; it names the CPU
# family, and the machine headers are gated on it:
#
#   sys/powerpc/include/spr.h:563   #if defined(AIM)  ... #elif defined(BOOKE)
#   sys/powerpc/include/pte.h:39    #if defined(AIM)  ... #else /* BOOKE */
#   sys/powerpc/include/tlb.h:33    #if defined(BOOKE_E500)
#
# config(8) turns them into options like any other - usr.sbin/config/
# mkoptions.cc:65, "Fake the cpu types as options" - so `cpu BOOKE_E500'
# is `#define BOOKE_E500 1' wherever sys/conf/options* declares the name.
#
# The first version of this index read only the files* lists a config
# pulls in with an explicit `files "..."' line, which is the DPAA
# fragment and almost nothing else: 315 sources. Every source named by
# sys/conf/files.<arch> got nothing, because no config mentions that
# list - config(8) reads it implicitly. Twelve of sweep 12's fourteen
# named powerpc ERRORs were that gap, all of them failing on an
# identifier inside an AIM or BOOKE block.
#
# So the question is asked properly: WHICH CONFIGS BUILD THIS FILE. A
# config's declared tokens are its `options', `device' and `cpu' names,
# transitively through `include', minus what `nooptions'/`nodevice'
# removes; a file's `optional' clause is satisfied when some alternative
# has all of its tokens; `standard' is satisfied always.
#
# Keyed by MACHINE_ARCH, because a cpu name belongs to the architecture
# whose configs declared it. sys/dev/xdma/xdma_sg.c is `optional xdma'
# and only riscv/GENERIC and riscv/QEMU declare that device, so the
# answer is RISCV - but arch_of() analyses that file as arm, where
# -DRISCV cost a translation unit that compiles today. One regression in
# a sample of sixty, and the whole reason this index is two-dimensional.
CONFIG_CPU = re.compile(r"^\s*cpu\s+([A-Za-z_][A-Za-z0-9_]*)")
# config(8) accepts both forms - usr.sbin/config/config.y:131 is
# `INCLUDE PATH' (quoted) and :136 is `INCLUDE ID' (bare) - and the
# quoted-only regex missed every one of the bare ones. `include
# GENERIC' is how all sixteen HARDENEDBSD configs in the tree are
# written, so each of them was read as if it declared nothing but its
# own overrides.
CONFIG_INCLUDE = re.compile(r'^\s*include\s+"?([^"\s]+)"?')
CONFIG_FILES = re.compile(r'^\s*files\s+"([^"]+)"')
CONFIG_DECL = re.compile(r"^\s*(?:device|options?)\s+([A-Za-z_][A-Za-z0-9_]*)")
CONFIG_NODECL = re.compile(
    r"^\s*(?:nodevice|nooptions?)\s+([A-Za-z_][A-Za-z0-9_]*)")
# usr.sbin/config/mkoptions.cc:96 - "Fake the value of MACHINE_ARCH as an
# option if necessary": config(8) sets the option whose name equals the
# config's machine_arch, case-insensitively, where options* declares one.
# `machine powerpc powerpc64' is the only reason
# sys/powerpc/pseries/mmu_phyp.c's `optional pseries powerpc64' is
# satisfiable at all; no config in the tree writes `options POWERPC64'.
CONFIG_MACHINE = re.compile(r"^\s*machine\s+(\S+)(?:\s+(\S+))?")
# The keywords that end a file's `optional' clause and begin its build
# instructions.
FILES_KEYWORD = ("compile-with", "no-obj", "no-depend", "dependency",
                 "clean", "warning", "before-depend", "local")


# (cpu names, the lower-case tokens it declares, the extra files* lists
# it names, its machine_arch).
Config = collections.namedtuple("Config", "cpus decl files march")


def _config_read(p: Path, root: Path, seen: set[Path]) -> Config:
    """One kernel config, read the way config(8) reads it.

    Both `include' and `files' name a path relative to the
    architecture's conf/ directory, never to the file doing the naming:
    sys/powerpc/conf/dpaa/config.dpaa says `files "dpaa/files.dpaa"'
    and means sys/powerpc/conf/dpaa/files.dpaa. Resolved against its own
    parent that is dpaa/dpaa/, which does not exist, and the DPAA
    configs then agreed on no cpu at all.
    """
    cpus: set[str] = set()
    decl: set[str] = set()
    files: set[Path] = set()
    march = ""
    if p in seen or not p.is_file():
        return Config(frozenset(), frozenset(), (), "")
    seen.add(p)
    for line in p.read_text(errors="replace").splitlines():
        line = line.split("#")[0]
        m = CONFIG_CPU.match(line)
        if m:
            cpus.add(m.group(1))
            decl.add(m.group(1).lower())
            continue
        m = CONFIG_FILES.match(line)
        if m:
            files.add((root / m.group(1)).resolve())
            continue
        m = CONFIG_INCLUDE.match(line)
        if m:
            c = _config_read((root / m.group(1)).resolve(), root, seen)
            cpus |= c.cpus
            decl |= c.decl
            files.update(c.files)
            march = march or c.march
            continue
        m = CONFIG_MACHINE.match(line)
        if m:
            march = march or (m.group(2) or m.group(1))
            decl.add((m.group(2) or m.group(1)).lower())
            continue
        m = CONFIG_NODECL.match(line)
        if m:
            decl.discard(m.group(1).lower())
            continue
        m = CONFIG_DECL.match(line)
        if m:
            decl.add(m.group(1).lower())
    return Config(frozenset(cpus), frozenset(decl),
                  tuple(sorted(files)), march)


def _clause(line: str, kind: str, opts: str) -> list[list[str]] | None:
    """The alternatives of one files* line, or None for `standard'."""
    if kind == "standard":
        return None
    toks: list[str] = []
    for tok in opts.split():
        if tok in FILES_KEYWORD:
            break
        toks.append(tok)
    return [a.split() for a in " ".join(toks).split("|")]


@functools.lru_cache(maxsize=None)
def _files_list(p: Path) -> tuple[tuple[str, tuple[tuple[str, ...], ...] | None], ...]:
    """(source, clause alternatives) for every .c a files* list names.

    `include "conf/files.x86"' inside sys/conf/files.amd64 is resolved
    against sys/, not against the list doing the naming.
    """
    out: list[tuple[str, tuple[tuple[str, ...], ...] | None]] = []
    if not p.is_file():
        return ()
    text = p.read_text(errors="replace").replace("\\\n", " ")
    for line in text.splitlines():
        # config(8) reads # as a comment, and sys/conf/files carries
        # commented-out source lines - `#ofed/drivers/...' among them -
        # that match the line shape exactly.
        line = line.split("#")[0]
        m = CONFIG_INCLUDE.match(line)
        if m:
            out.extend(_files_list((SYS / m.group(1)).resolve()))
            continue
        m = FILES_OPTIONAL.match(line)
        if not m:
            continue
        alts = _clause(line, m.group("kind"), m.group("opts"))
        out.append(("sys/" + m.group("src"),
                    None if alts is None
                    else tuple(tuple(a) for a in alts)))
    return tuple(out)


def _satisfied(decl: frozenset[str],
               alts: tuple[tuple[str, ...], ...] | None) -> bool:
    """config(8)'s reading of an `optional' clause."""
    if alts is None:
        return True
    for a in alts:
        if not a:
            return True
        if all((t[1:].lower() not in decl) if t.startswith("!")
               else (t.lower() in decl) for t in a):
            return True
    return False


@functools.lru_cache(maxsize=None)
def _config_index() -> dict[tuple[str, str], tuple[frozenset[str], ...]]:
    """(source, machine_arch) -> the cpu set of each config that builds it."""
    per: dict[tuple[str, str], list[frozenset[str]]] = {}
    for arch_dir in sorted(set(SYS_DIR.values())):
        confd = SYS / arch_dir / "conf"
        base = [SYS / "conf" / "files", SYS / "conf" / f"files.{arch_dir}"]
        dflt = _config_read(confd / "DEFAULTS", confd, set())
        for cf in sorted(confd.rglob("*")):
            # A kernel config is the ALL-CAPS file; DEFAULTS, NOTES and
            # the files.*/std.*/config.* fragments are not kernels.
            if not cf.is_file() or not cf.name.isupper():
                continue
            if cf.name in ("DEFAULTS", "NOTES"):
                continue
            c = _config_read(cf, confd, set())
            if not c.cpus and not c.decl:
                continue
            cpus = c.cpus | dflt.cpus
            decl = c.decl | dflt.decl
            march = c.march or dflt.march or arch_dir
            for fl in base + sorted(set(c.files) | set(dflt.files)):
                for rel, alts in _files_list(fl.resolve()):
                    if _satisfied(decl, alts):
                        per.setdefault((rel, march), []).append(cpus)
    return {k: tuple(v) for k, v in per.items()}


@functools.lru_cache(maxsize=None)
def files_cpu_index() -> dict[str, dict[str, tuple[str, ...]]]:
    """source -> machine_arch -> the cpu names EVERY config building it sets.

    Intersection, not union: a cpu only some of them declare is not one
    this file may assume, and inventing it would be the analyser
    checking a kernel nobody builds.
    """
    out: dict[str, dict[str, tuple[str, ...]]] = {}
    for (rel, march), sets in _config_index().items():
        common = frozenset.intersection(*sets)
        if common:
            out.setdefault(rel, {})[march] = tuple(sorted(common))
    return out


@functools.lru_cache(maxsize=None)
def files_cpu_alternatives() -> dict[str, dict[str, tuple[tuple[str, ...], ...]]]:
    """source -> machine_arch -> each cpu set a config building it declares.

    The intersection is the only part that is not a guess, and for a
    `standard' file it is often empty: sys/powerpc/powerpc/trap.c is
    built by every powerpc kernel, half of them AIM and half BOOKE, and
    its frame_is_trap_inst() reads frame->cpu.booke.esr in the #else of
    an `#ifdef AIM'. With neither macro the BOOKE arm is compiled and
    ESR_PTR is undeclared, because sys/powerpc/include/spr.h:713 puts it
    inside `#if defined(BOOKE)'. This is what analyze.py retries with -
    the same rule the option retry follows, and never for a file that
    already compiled.
    """
    out: dict[str, dict[str, tuple[tuple[str, ...], ...]]] = {}
    for (rel, march), sets in _config_index().items():
        common = frozenset.intersection(*sets)
        distinct = sorted({tuple(sorted(s)) for s in sets if s})
        if distinct and (len(distinct) > 1 or not common):
            out.setdefault(rel, {})[march] = tuple(distinct)
    return out


# config(8) reads sys/<arch>/conf/DEFAULTS before any kernel config, so
# its options hold for every kernel that architecture builds. arm64's
# has INTRNG, and without it nine translation units hit
#
#   #error Need INTRNG for this file
#
# or lose intr_ipi_pic_register(). Six architectures, 33 options between
# them, and the file is two dozen lines: read it rather than list them.
SYS_DIR = {"amd64": "amd64", "aarch64": "arm64", "armv7": "arm",
           "i386": "i386", "powerpc64": "powerpc", "riscv64": "riscv"}
# `device' as well as `options', because config(8) reads both out of
# DEFAULTS and turns a device into DEV_<NAME> where sys/conf/options
# declares one. sys/x86/isa/atrtc.c is `standard' - in every x86 kernel
# - and its `#include <isa/isavar.h>' is inside `#ifdef DEV_ISA' while
# four uses of what that header declares are not, so without the macro
# `ISA_PNP_INFO(atrtc_ids);' at :683 parses as a function declaration
# with an untyped parameter. NO amd64 or i386 config declares `device
# isa' directly; DEFAULTS:10 does, for all of them.
DEFAULTS_OPT = re.compile(
    r"^\s*(options?|device)\s+([A-Za-z_][A-Za-z0-9_]*)")


# ...and sys/conf/Makefile.<arch>, which the kernel build reads for
# every file and this tool never opened. All six add to INCLUDES:
#
#   Makefile.amd64:32    INCLUDES+= -I$S/contrib/libfdt
#   Makefile.arm:30      INCLUDES+= -I$S/contrib/libfdt \
#                                   -I$S/contrib/device-tree/include \
#                                   -I$S/dts/include
#
# libfdt's own headers include <fdt.h> with ANGLE brackets and the file
# is sys/contrib/libfdt/fdt.h, so the directory itself has to be on the
# path - which those six lines do and nothing else in the build does.
# Five translation units failed on exactly that: dev/ofw/ofw_fdt.c,
# arm and arm64's machdep_boot.c, powerpc/ofw/ofw_machdep.c and
# riscv/riscv/machdep.c.
#
# CFLAGS is read for its -D and -I only. The rest of what those files
# add is code generation - -msoft-float, -mabi=spe, -fPIC,
# -fno-omit-frame-pointer - which is the compiler's business and not
# the analyser's, and CFLAGS.gcc / CFLAGS.clang lines are a choice of
# compiler this tool has already made.
ARCH_MK_ADD = re.compile(r"^\s*(INCLUDES|CFLAGS)\s*\+?=\s*(.*)$")


@functools.lru_cache(maxsize=None)
def arch_makefile_flags(arch: str) -> tuple[str, ...]:
    """The -I and -D sys/conf/Makefile.<arch> adds for every kernel file."""
    d = SYS_DIR.get(arch)
    if not d:
        return ()
    p = SYS / "conf" / f"Makefile.{d}"
    if not p.is_file():
        return ()
    out: list[str] = []
    text = p.read_text(errors="replace").replace("\\\n", " ")
    for line in text.splitlines():
        m = ARCH_MK_ADD.match(line.split("#")[0])
        if not m:
            continue
        for w in m.group(2).split():
            if w.startswith("-I"):
                path = w[2:].replace("$S", str(SYS)).replace("${SRCTOP}",
                                                             str(SRC))
                if "$" not in path and Path(path).is_dir():
                    out.append(f"-I{path}")
            elif w.startswith("-D") and "$" not in w and len(w) > 2:
                out.append(w)
    return tuple(dict.fromkeys(out))


@functools.lru_cache(maxsize=None)
def defaults_options(arch: str) -> tuple[str, ...]:
    """-D for every option in this architecture's conf/DEFAULTS."""
    d = SYS_DIR.get(arch)
    if not d:
        return ()
    p = SYS / d / "conf" / "DEFAULTS"
    if not p.is_file():
        return ()
    out = []
    for line in p.read_text(errors="replace").splitlines():
        line = line.split("#")[0]
        m = DEFAULTS_OPT.match(line)
        if not m:
            continue
        if m.group(1) == "device":
            # A device only becomes a macro where options* declares its
            # DEV_<NAME>; `device mem' and `device io' do not.
            spelled = _declared_options().get("dev_" + m.group(2).lower())
            if spelled:
                out.append(f"-D{spelled}")
        else:
            out.append(f"-D{m.group(2)}")
    return tuple(dict.fromkeys(out))


# An architecture can also be stated by the OPTION a file is `optional'
# on rather than by the files* it is in. sys/conf/files - the
# architecture-neutral one - carries
#
#   contrib/alpine-hal/al_hal_iofic.c  optional al_iofic ...
#
# and `device al_eth' / `device al_iofic' appear in exactly one place in
# the tree, sys/arm64/conf/std.al. Analysed as amd64 those eleven files
# report twenty errors on `dsb', `dmb' and the rest of arm64's barrier
# intrinsics; as arm64 they compile clean. The option is the only thing
# that says so.
# The \s+ AFTER the keyword meant a `standard' line with nothing
# following it never matched at all - arm/arm/sp804.c and 578 others.
FILES_OPTIONAL = re.compile(
    r"^(?P<src>\S+\.c)\s+(?P<kind>optional|standard)\b(?P<opts>[^\\]*)")
CONF_DECL = re.compile(r"^\s*(?:device|options?)\s+([A-Za-z_][A-Za-z0-9_]*)",
                       re.M)


@functools.lru_cache(maxsize=None)
def _option_arches() -> dict[str, frozenset[str]]:
    """option or device name -> the architectures whose configs declare it.

    Keyed in LOWER CASE, because config(8) is: a kernel configuration
    writes `options FDT' and sys/conf/files writes `optional ... fdt',
    and they are the same option. Keying on the spelling made `fdt'
    resolve to riscv64 alone - the one architecture whose config happens
    to say `device fdt' in lower case - so every FDT driver's option set
    intersected to nothing and the index had no opinion about any of
    them.
    """
    out: dict[str, set[str]] = {}
    for arch, d in sorted(SYS_DIR.items()):
        confd = SYS / d / "conf"
        if not confd.is_dir():
            continue
        for cf in sorted(confd.rglob("*")):
            # NOTES lists every option the architecture COULD have, which
            # is not the same as one a kernel is built with, and taking
            # it makes every option ambiguous.
            if not cf.is_file() or cf.name == "NOTES":
                continue
            for m in CONF_DECL.finditer(cf.read_text(errors="replace")):
                out.setdefault(m.group(1).lower(), set()).add(arch)
    return {k: frozenset(v) for k, v in out.items()}


@functools.lru_cache(maxsize=None)
def _declared_options() -> dict[str, str]:
    """lower-case option name -> the spelling sys/conf/options* uses.

    config(8) turns each of these into a #define in the opt_*.h named on
    the same line - or into opt_global.h when no header is named - and
    sys/conf/files refers to the same option in lower case. Keyed for the
    lower-case lookup and valued with the spelling that becomes the macro.
    """
    out: dict[str, str] = {}
    for f in sorted(SYS.glob("conf/options*")):
        if not f.is_file():
            continue
        for line in f.read_text(errors="replace").splitlines():
            line = line.split("#")[0].strip()
            m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)\b", line)
            if m:
                out.setdefault(m.group(1).lower(), m.group(1))
    # config(8) also writes a macro for a DEVICE. `device acpi' becomes
    # `#define DEV_ACPI 1' in the header the DEV_ACPI line of
    # sys/conf/options names - :711, opt_acpi.h. The token in files* and
    # in a config is `acpi'; the macro is DEV_ACPI; a lookup on the name
    # alone misses it, and sys/arm64/include/intr.h:45 is
    #
    #   #ifdef DEV_ACPI
    #   #define ACPI_MSI_XREF   2
    #   #define ACPI_GPIO_XREF  3
    #
    # so pl061_acpi.c, vmbus_pcib.c and pci_host_generic_acpi.c failed on
    # an undeclared identifier while machine/intr.h sat on their path
    # with the definition guarded out. The device name maps to its
    # DEV_<NAME> only where options* declares one, which is 18 of the
    # 929: it is not a rule about every device, it is the list config(8)
    # keeps.
    for k in list(out):
        if k.startswith("dev_"):
            out.setdefault(k[4:], out[k])
    return out


@functools.lru_cache(maxsize=None)
def files_option_defines() -> dict[str, tuple[str, ...]]:
    """source under sys/ -> the -D its own `optional' clause implies.

    opt_shim() writes EMPTY opt_*.h, which is what config(8) writes for an
    option that is NOT set. That is right for a file the kernel builds
    either way, and wrong for one whose `optional' clause NAMES the
    option: sys/dev/random/fenestrasX/fx_brng.c is `optional
    !random_loadable random_fenestrasx', so the only configuration that
    compiles it has RANDOM_FENESTRASX set - and sys/sys/vdso.h:96 declares
    fxrng_push_seed_generation() inside `#ifdef RANDOM_FENESTRASX', so
    without it the file fails on a function that is not declared.

    Only tokens sys/conf/options* declares become a -D, because those are
    exactly the ones config(8) turns into a #define; a `device' name is
    not one. A negated token (`!random_loadable') is left unset, which is
    what the shim already does. For a disjunction the FIRST alternative is
    taken - it is a configuration that builds the file, and taking all of
    them would be a configuration that may not exist.
    """
    opts = _declared_options()
    out: dict[str, tuple[str, ...]] = {}
    for mk in sorted(SYS.rglob("files*")):
        if not mk.is_file() or mk.suffix in (".c", ".h"):
            continue
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_OPTIONAL.match(line)
            if not m:
                continue
            toks: list[str] = []
            for tok in m.group("opts").split():
                if tok in ("compile-with", "no-obj", "no-depend",
                           "dependency", "clean", "warning",
                           "before-depend", "local"):
                    break
                toks.append(tok)
            # A DISJUNCTION means the build has several configurations
            # that compile this file, and picking one of them is a
            # guess. Take the INTERSECTION - the tokens every
            # alternative names - and define nothing when they share
            # none.
            #
            # Taking the first alternative instead cost six regressions
            # in sweep 11, all of them sys/xdr:
            #
            #   xdr/xdr.c  optional xdr | krpc | nfslockd | nfscl |
            #                       nfsd | zfs
            #
            # `XDR' is a declared option (sys/conf/options:489, bare,
            # so opt_global.h) that no configuration in the tree sets,
            # and sys/rpc/xdr.h:299 is
            #
            #   extern bool_t xdr_u_int(XDR *, u_int *);
            #
            # where XDR is a TYPE. `#define XDR 1' turns every one of
            # those declarations into a syntax error. The six files
            # went from OK to ERROR on a -D chosen from an alternative
            # nothing uses, in a header the option's own subsystem
            # defines.
            #
            # The intersection keeps what is not a guess:
            # fx_brng.c's `optional !random_loadable random_fenestrasx'
            # is one alternative and keeps -DRANDOM_FENESTRASX; and
            # tcp_ratelimit.c's `optional ratelimit inet | ratelimit
            # inet6' keeps -DRATELIMIT, which is the one both agree on,
            # and drops the -DINET that only one of them names.
            alts = [a.split() for a in " ".join(toks).split("|")]
            common = set(alts[0])
            for a in alts[1:]:
                common &= set(a)
            defs = tuple(f"-D{opts[t.lower()]}" for t in alts[0]
                         if t in common and not t.startswith("!")
                         and t.lower() in opts)
            if defs:
                out.setdefault("sys/" + m.group("src"), defs)
    return out


@functools.lru_cache(maxsize=None)
def files_option_alternatives() -> dict[str, tuple[tuple[str, ...], ...]]:
    """source under sys/ -> the -D each alternative of its clause implies.

    files_option_defines() answers with the intersection, which is the
    only part that is not a guess. Some files need a guess to compile at
    all: sys/arm/arm/debug_monitor.c is `optional ddb | gdb' and defines
    dbg_monitor_init() twice, once inside `#ifdef DDB' and once outside,
    so with neither macro it is a redefinition. This is what analyze.py
    retries with when the intersection will not build - never for a file
    that already compiled, which is the same rule the architecture retry
    follows and for the same reason.
    """
    opts = _declared_options()
    out: dict[str, tuple[tuple[str, ...], ...]] = {}
    for mk in sorted(SYS.rglob("files*")):
        if not mk.is_file() or mk.suffix in (".c", ".h"):
            continue
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_OPTIONAL.match(line)
            if not m:
                continue
            toks: list[str] = []
            for tok in m.group("opts").split():
                if tok in ("compile-with", "no-obj", "no-depend",
                           "dependency", "clean", "warning",
                           "before-depend", "local"):
                    break
                toks.append(tok)
            alts = [a.split() for a in " ".join(toks).split("|")]
            if len(alts) < 2:
                continue
            got = []
            for a in alts:
                d = tuple(f"-D{opts[x.lower()]}" for x in a
                          if not x.startswith("!") and x.lower() in opts)
                if d and d not in got:
                    got.append(d)
            if got:
                out.setdefault("sys/" + m.group("src"), tuple(got))
    return out


@functools.lru_cache(maxsize=None)
def files_opt_arch_index() -> dict[str, str]:
    """source under sys/ -> the one architecture its options allow.

    Intersection over the file's options, like files_cpu_index: a file
    that needs `al_iofic' AND `fdt' can only be built where both exist.

    ALL the survivors are returned, sorted, not just the first: `fdt' and
    `gpioregulator' between them leave aarch64 and armv7, and
    sys/dev/gpio/gpioregulator.c compiles as armv7 and not as aarch64.
    Picking one was picking wrong half the time, so the caller retries
    each in turn.

    This is a HINT, not an answer, and arch_of() deliberately does not
    use it. "No amd64 config declares this device" is not "amd64 cannot
    build this file": sys/dev/nvmem/nvmem.c is `optional nvmem', which
    only the three FDT architectures declare, and it compiles clean as
    amd64. Inferring an architecture from that would re-interpret 272
    translation units that were already being read correctly, which is
    inventing a build rather than reading one.

    So analyze.py uses it only where the default FAILED: a file that
    does not compile as amd64 is retried against the architecture the
    build system says can build it, and the better of the two results is
    the one reported. Empirical, and it cannot touch a file that already
    compiled.
    """
    opts = _option_arches()
    out: dict[str, str] = {}
    for mk in sorted(SYS.rglob("files*")):
        if not mk.is_file() or mk.suffix in (".c", ".h"):
            continue
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = FILES_OPTIONAL.match(line)
            if not m:
                continue
            # `optional miibus | e1000phy' is a DISJUNCTION: the file is
            # built where either holds. Intersecting the two gave armv7,
            # because only sys/arm/conf declares e1000phy - and made a
            # PHY driver every architecture builds look like ARM code.
            # Union over the alternatives, intersection within each.
            toks: list[str] = []
            for tok in m.group("opts").split():
                if tok in ("compile-with", "no-obj", "no-depend", "dependency",
                           "clean", "warning", "before-depend", "local"):
                    break
                toks.append(tok)
            cands: set[str] = set()
            for alt in " ".join(toks).split("|"):
                inner: frozenset[str] | None = None
                for tok in alt.split():
                    a = opts.get(tok.lower())
                    if a is None:
                        continue
                    if inner is not None and not (inner & a):
                        # The tokens disagree, and a file the build lists
                        # is built SOMEWHERE, so an empty intersection is
                        # this index being incomplete rather than an
                        # answer. Keep the union: the retry wants
                        # candidates, and none is worse than several.
                        inner = inner | a
                        continue
                    inner = a if inner is None else (inner & a)
                if inner is None:
                    # An alternative with no known option constrains
                    # nothing, so the whole line constrains nothing.
                    cands = set()
                    break
                cands |= inner
            if cands and "amd64" not in cands:
                out.setdefault("sys/" + m.group("src"), tuple(sorted(cands)))
    return out


# lib/msun/ld80 and lib/msun/ld128 name a long double FORMAT, not an
# architecture, and the tree says which architectures have which:
#
#   lib/msun/aarch64/Makefile.inc:1   LDBL_PREC = 113
#   lib/msun/amd64/Makefile.inc:8     LDBL_PREC = 64
#   lib/msun/Makefile:24-29           .if ${LDBL_PREC} == 64
#                                     .PATH: ${.CURDIR}/ld80
#                                     .elif ${LDBL_PREC} == 113
#                                     .PATH: ${.CURDIR}/ld128
#
# Analysed as amd64, whose long double is 80-bit, the five ld128 sources
# fail; as aarch64 or riscv64 they compile clean. Read the mapping out
# of those Makefile.inc rather than writing "ld128 means aarch64" here,
# because the tree is where it is decided and a copy drifts.
LDBL_DIR = {64: "ld80", 113: "ld128"}


@functools.lru_cache(maxsize=None)
def _ldbl_arch() -> dict[str, str]:
    """ld80/ld128 -> an architecture key whose long double is that wide."""
    out: dict[str, str] = {}
    inv = {v: k for k, v in ARCH_DIR.items()}
    for mk in sorted((SRC / "lib" / "msun").glob("*/Makefile.inc")):
        m = re.search(r"^LDBL_PREC\s*=\s*(\d+)", mk.read_text(errors="replace"),
                      re.M)
        if not m:
            continue
        d = LDBL_DIR.get(int(m.group(1)))
        if not d:
            continue
        # msun's i387 is i386's directory; the rest match ARCH_DIR.
        name = mk.parent.name
        arch = inv.get(name) or ("i386" if name == "i387" else None)
        if arch:
            out.setdefault(d, arch)
    return out


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
    # lib/libc/softfloat is libc's software floating point, and amd64
    # does not build a word of it - softfloat/Makefile.inc is included
    # only from arm, powerpc, powerpc64, powerpcspe and riscv. Analysed
    # against amd64 it cannot find <milieu.h>, because amd64 has no
    # lib/libc/amd64/softfloat/ for it to come from. It is real code
    # that five of PBSD's six architectures run for every float
    # operation; the arch it is checked against just has to be one that
    # builds it. arm carries SOFTFLOAT_BITS=32 and everything else 64,
    # so the bits64/ sources are named for a 64-bit one.
    if rel.startswith("lib/libc/softfloat/"):
        return "riscv64" if "/bits64/" in rel else "armv7"
    if len(parts) > 2 and parts[0] == "lib" and parts[1] == "msun":
        cand = _ldbl_arch().get(parts[2])
        if cand:
            return cand
    if len(parts) > 2 and parts[0] == "lib" and parts[1] in ("libc", "msun"):
        cand = ARCH_DIR.get(parts[2])
        if cand:
            return cand
    # ...and then it was still only sys/<arch>/, which is not where the
    # kernel keeps all of its per-architecture code:
    #
    #   sys/cddl/dev/fbt/riscv/fbt_isa.c          <machine/riscvreg.h>
    #   sys/cddl/dev/dtrace/aarch64/dtrace_subr.c <machine/armreg.h>
    #   sys/cddl/contrib/opensolaris/uts/powerpc/dtrace/fasttrap_isa.c
    #
    # all asked amd64 for a header only their own architecture has. Any
    # component that names an architecture names it, so take the DEEPEST
    # one: sys/cddl/dev/dtrace/i386/ is i386, and the sys/ prefix is not
    # a claim about anything.
    if parts[0] == "sys":
        for name in reversed(parts[1:-1]):
            cand = SYS_ARCH.get(name)
            if cand:
                return cand
    # ...and neither was the run-time linker, which keeps its relocation
    # processor per architecture under libexec/rtld-elf/<arch>/reloc.c.
    # All seven were analysed against amd64's <machine/*.h>. Five would
    # not compile and said so; the interesting two are amd64's, right by
    # coincidence of being amd64, and riscv's, which COMPILED - a clean
    # check of a program riscv does not build. That the one finding it
    # produced (a leaked symbol cache) is architecture-neutral C is luck.
    #
    # Enumerating the third place was the wrong lesson: a fourth was
    # already waiting (lib/csu, lib/libsys, stand, usr.sbin/bhyve, and
    # contrib/arm-optimized-routines, 357 files in 79 directories). The
    # rule the tree actually follows is the one the sys/ branch above
    # already uses - the DEEPEST component that names an architecture is
    # the architecture - so it is applied everywhere rather than to a
    # list of prefixes that will be short by one again next time.
    for name in reversed(parts[:-1]):
        cand = ARCH_DIR.get(name) or SYS_ARCH.get(name)
        if cand:
            return cand
    if parts[0] == "sys":
        cand = files_arch_index().get(rel)
        if cand:
            return cand
    return default


_GEN_VAR = re.compile(r"\$\{[A-Za-z_.][A-Za-z0-9_.]*\}")


@functools.lru_cache(maxsize=None)
def gen_headers() -> str:
    """The headers sys/conf/files says the build generates, generated.

    Nineteen entries in sys/conf/files are not sources but RECIPES: a
    header name, a `dependency', a `compile-with' naming the script that
    writes it, and `before-depend' saying it happens before anything is
    compiled. bhnd_nvram_map.h and snd_fxdiv_gen.h are two of them, and
    sixteen translation units in sys/dev failed on nothing but their
    absence - `fatal error: bhnd_nvram_map.h file not found' - which is
    the same shape as device_if.h above and has the same answer.

    So the recipe is read and run, rather than the header being stubbed.
    A stub would be a guess about a file the tree tells you how to make;
    running the tree's own generator on the tree's own input gives the
    declarations the kernel compiles against.

    $S, ${SRCTOP}, ${AWK} and ${CPP} are substituted because the build
    sets them, and anything left is expanded to NOTHING - which is what
    bmake does with a variable no makefile defines. That is not a guess:
    ${FEEDER_EQ_PRESETS} is genuinely unset in this tree, and the awk
    script's own default is what the shipped header contains.

    Where the variable does carry a kernel configuration's answer -
    ${SC_DFLT_FONT}, ${KEYMAP}, ${FDT_DTS_FILE}, ${.TARGET} - expanding
    it away makes the recipe fail rather than write something wrong: the
    redirect has no filename, or uudecode has no input. So the header is
    simply absent and the files needing it stay ERROR, which is the
    honest report. The result is checked rather than assumed: a recipe
    that exits 0 without producing the header it names is treated as
    having failed.
    """
    d = Path(tempfile.mkdtemp(prefix="pbsd_gen_"))
    f = SYS / "conf" / "files"
    if not f.is_file():
        return d.as_posix()
    text = f.read_text(errors="replace").replace("\\\n", " ")
    for line in text.splitlines():
        if not re.match(r"^\S+\.h\s", line) or "before-depend" not in line:
            continue
        m = re.search(r'compile-with\s+"([^"]*)"', line)
        if not m:
            continue
        hdr = line.split()[0]
        cmd = (m.group(1).replace("$S", str(SYS))
               .replace("${SRCTOP}", str(SRC))
               .replace("${AWK}", "awk").replace("${CPP}", "cpp"))
        cmd = _GEN_VAR.sub("", cmd)
        try:
            subprocess.run(["sh", "-c", cmd], cwd=d, check=True,
                           capture_output=True, timeout=120)
        except (OSError, subprocess.SubprocessError):
            # One recipe that will not run is one header still missing,
            # not a reason to lose the others.
            pass
        out = d / hdr
        if out.exists() and out.stat().st_size == 0:
            # An empty file is worse than none: it satisfies the #include
            # and then every declaration in it is absent, which reports as
            # a wall of unrelated errors.
            out.unlink()
    return d.as_posix()


@functools.lru_cache(maxsize=None)
def _component_dir(rel: str) -> Path | None:
    """The directory bmake would be run in to build this source.

    The nearest ancestor with a Makefile, stopping at the scope root -
    lib/libc/stdio/fopen.c is built by a make in lib/libc, because
    lib/libc/stdio holds a Makefile.inc and no Makefile. Returns None
    for a source with no Makefile above it at all, which is a source
    nothing builds.
    """
    top = rel.split("/")[0]
    if top not in USERLAND_TOP:
        return None
    d = (SRC / rel).parent
    root = SRC / top
    while True:
        if (d / "Makefile").is_file():
            return d
        if d == root or root not in d.parents:
            return None
        d = d.parent


@functools.lru_cache(maxsize=None)
def incs_shim(arch: str = "amd64") -> str:
    """/usr/include, as the tree's own Makefiles say to build it.

    A program compiles against <devstat.h>, <netgraph.h>, <jail.h>,
    <security/pam_appl.h>. Every one of those is in the tree and none of
    them is beside the program: the real build reaches them through
    /usr/include, because `make installworld' put them there first. The
    analyser has no installed tree, and ten of one shard's unlisted
    ERRORs were one such header each.

    The obvious fix - add the library's source directory to -I - is a
    guess in two directions at once. It puts every private header in
    that directory on the path as well, and it gets the installed
    LAYOUT wrong: <security/pam_appl.h> lives in
    contrib/openpam/include/security, <sha256.h> in sys/crypto/sha2,
    and neither is found by -I on a lib directory.

    So the layout is built rather than guessed. Each Makefile in the
    tree is asked what it installs and where -

        INCS=       devstat.h
        INCSDIR=    ${INCLUDEDIR}

    - and this reproduces the answer as a tree of symlinks, 2,079 of
    them, rooted at one directory that goes on -I. The result is the
    include path a program is actually compiled against, and no more:
    a header no Makefile installs is not on it, which is the same
    answer the build gives.
    """
    d = Path(tempfile.mkdtemp(prefix="pbsd_incs_"))
    try:
        headers = userland_names.installed_headers(arch)
    except Exception:
        return d.as_posix()
    skip = str(SRC / "include") + "/"
    for installed, source in headers.items():
        # include/'s own headers are already reached through -I on the
        # tree's include directory, in the position the rest of this
        # function puts it. Linking them in here as well would put a
        # second copy of <stdio.h> and <sys/*.h> EARLIER on the path
        # than lib/libc/include, which is an ordering the build chose -
        # and flag order is not cosmetic (see kern.pre.mk and openzfs's
        # condvar.h). Only what lives outside include/ is new here.
        if source.startswith(skip):
            continue
        dst = d / installed
        try:
            dst.parent.mkdir(parents=True, exist_ok=True)
            if not dst.exists():
                dst.symlink_to(source)
        except OSError:
            pass
    return d.as_posix()


@functools.lru_cache(maxsize=None)
def rpc_headers() -> str:
    """The rpcsvc headers the build generates, generated the same way.

    Thirteen of one sweep shard's unlisted ERRORs were one missing
    header each: rpcsvc/rquota.h under libexec/rpc.rquotad,
    rpcsvc/rstat.h under rpc.rstatd, rnusers.h, rwall.h, spray.h, and
    rpcsvc/yp.h five times under libexec/ypxfr. None of them is in the
    tree, and none of them is meant to be - include/rpcsvc/Makefile
    lists twenty-two .x interface definitions and one suffix rule:

        .x.h:
            ${RPCCOM} -h -DWANT_NFS3 ${.IMPSRC} -o ${.TARGET}
        RPCCOM= RPCGEN_CPP=${CPP:Q} rpcgen -C

    which is the same shape as sys/conf/files' before-depend recipes,
    and gets the same answer: run the tree's recipe on the tree's input
    rather than stub the header. Both HDRS and the rule are read out of
    that Makefile, so an interface added to it is generated without
    this function being touched.

    One substitution is not the tree's, and is worth naming: rpcgen
    itself is the host's, not usr.bin/rpcgen. Both implement the same
    RFC 1831 mapping from a .x to its declarations, and the header is
    wanted here only so the declarations exist - nothing in it is
    compiled into anything that runs. A recipe that exits non-zero, or
    exits 0 without writing the header it names, leaves that header
    absent and its callers ERROR, which is the honest report.
    """
    d = Path(tempfile.mkdtemp(prefix="pbsd_rpc_"))
    out = d / "rpcsvc"
    out.mkdir()
    mk = SRC / "include" / "rpcsvc" / "Makefile"
    if not mk.is_file() or not shutil.which("rpcgen"):
        return d.as_posix()
    text = mk.read_text(errors="replace").replace("\\\n", " ")
    m = re.search(r"^HDRS=\s*(.*)$", text, re.M)
    r = re.search(r"^\.x\.h:\s*\n\t(.*)$", text, re.M)
    if not m or not r:
        return d.as_posix()
    recipe = (r.group(1).replace("${RPCCOM}", "rpcgen -C")
              .replace("${.IMPSRC}", "{src}").replace("${.TARGET}", "{dst}"))
    env = dict(os.environ, RPCGEN_CPP="cpp")
    for hdr in m.group(1).split():
        if not hdr.endswith(".h"):
            continue
        x = SRC / "include" / "rpcsvc" / (hdr[:-2] + ".x")
        if not x.is_file():
            continue
        cmd = recipe.format(src=str(x), dst=str(out / hdr))
        try:
            subprocess.run(["sh", "-c", cmd], cwd=SRC / "include" / "rpcsvc",
                           check=True, capture_output=True, timeout=60,
                           env=env)
        except (OSError, subprocess.SubprocessError):
            pass
        f = out / hdr
        if f.exists() and f.stat().st_size == 0:
            f.unlink()
    return d.as_posix()


@functools.lru_cache(maxsize=None)
def iface_shim(arch: str = "amd64") -> str:
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

    # Two interfaces in this tree share a basename, and the shim is one
    # flat directory, so the second one written wins:
    #
    #   sys/kern/pic_if.m               14 methods, the INTRNG interface
    #   sys/powerpc/powerpc/pic_if.m     9 methods, PowerPC's own
    #
    # arm64 got PowerPC's, and every arm64 interrupt controller failed on
    # `unknown type name pic_disable_intr_t' - 37 translation units of
    # "too many errors emitted", which is what a cascade from ten unknown
    # types looks like. Skip the .m files that belong to a DIFFERENT
    # architecture; sys/kern's and the architecture's own both stay, and
    # the architecture's is written last so it wins where they collide,
    # which is what config(8) does with .PATH.
    mine = SYS_DIR.get(arch, "amd64")
    others = {v for k, v in SYS_DIR.items() if v != mine} | {"powerpcspe"}
    ms = [m for m in (SRC / "sys").rglob("*_if.m")
          if m.relative_to(SRC / "sys").parts[0] not in others]
    ms.sort(key=lambda m: m.relative_to(SRC / "sys").parts[0] == mine)
    for m in ms:
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
    # Both spellings. sys/netinet/cc/cc.c:52 is the tree's only
    #
    #   #include <opt_cc.h>
    #
    # and matching only the quoted form left the whole congestion-control
    # framework - the file every cc_*.c algorithm hangs off - reporting
    # "'opt_cc.h' file not found" and contributing nothing. One file, and
    # one character of regex.
    pat = re.compile(r'#\s*include\s+["<](opt_[A-Za-z0-9_]+\.h)[">]')
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


def _dedupe_defines(flags: list[str]) -> list[str]:
    """Drop a -DNAME whose NAME an earlier flag already defined.

    A file can be named by a sys/conf/files line AND sit under a module
    Makefile's .PATH, and the two do not agree, because a real build
    compiles it into the kernel OR into the module and never both:
    sys/conf/kern.pre.mk:208 passes -DWITH_NETDUMP for the in-kernel
    ZFS and sys/modules/zfs/Makefile:38 passes -DWITHOUT_NETDUMP.
    Merged, clang takes the last one on the line, so which of the two
    won depended on the order this file happens to assemble them in.

    An -I is additive and an unused one costs nothing, so those still
    merge. A -D is exclusive per name, and the earlier source wins: the
    analyser's own -D_KERNEL first, then what the file's own
    compile-with says, then the module's.
    """
    out: list[str] = []
    seen: set[str] = set()
    for f in flags:
        if f.startswith("-D"):
            name = f[2:].split("=", 1)[0].split("(", 1)[0]
            if name in seen:
                continue
            seen.add(name)
        out.append(f)
    return out


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


def include_flags(src: Path, arch: str = "amd64", cc: str = "clang",
                  opts: tuple[str, ...] | None = None,
                  cpu: tuple[str, ...] | None = None) -> list[str]:
    """-nostdinc plus everything that source needs, in build order.

    `opts' overrides the -D a file's own `optional' clause implies. The
    default - None - is the no-guess answer: for a disjunction, only the
    tokens every alternative names. analyze.py's retry passes one
    alternative's full set when that answer will not compile, the same
    way it passes another architecture. See files_option_alternatives().
    """
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
    #
    # -DHARDENEDBSD is the same class of mistake one macro over.
    # share/mk/sys.mk:339 does `.include <bsd.hardenedbsd.mk>', whose
    # entire content is
    #
    #     CFLAGS+=   -DHARDENEDBSD
    #     CXXFLAGS+= -DHARDENEDBSD
    #
    # and sys.mk is the root of the whole build, so every translation
    # unit that ships is compiled with it. This sweep was not, so every
    # `#ifdef HARDENEDBSD' block took its false branch here and its true
    # branch in the shipped binary.
    #
    # Three files tree-wide key on it -- libexec/rtld-elf/rtld.c,
    # usr.bin/gcore/gcore.c and tests/sys/kern/kern_copyin.c -- so this
    # is not the 607-file blast radius the __FreeBSD__ fix had. It is
    # worth the flag anyway, because one of the three is the run-time
    # linker, and the block it hides is the one that cost thirty-seven
    # boot runs.
    flags = ["-nostdinc",
             *target_flags(arch, cc),
             "-U__linux__", "-U__gnu_linux__", "-D__FreeBSD__=15",
             "-DHARDENEDBSD",
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

        # What the module Makefile and the compile-with say, and it goes
        # HERE rather than at the end. sys/conf/kmod.mk:128 is
        #
        #   CFLAGS:= ${CFLAGS:N-I*} ${NOSTDINC} ${INCLMAGIC} ${CFLAGS:M-I*}
        #
        # which exists to hold every -I the module Makefile wrote, in
        # its own order, and line 139 then appends
        # `-I. -I${SYSDIR} -I${SYSDIR}/contrib/ck/include' AFTER them.
        # So a module's include directories precede -I$S in the real
        # build, and appending them last here is the reverse of what the
        # tree does. It cost four files that need opensolaris's
        # <sys/types.h> - uint_t, hrtime_t, uio_t - rather than
        # FreeBSD's: sdt.c, nfs_clkdtrace.c, opensolaris_uio.c and
        # ctf_mod.c, all of them named by a module whose CFLAGS say so.
        #
        # A module's SRCS are named relative to its .PATH, so one .PATH
        # covers a whole subtree: sys/modules/qat/qat_api takes .PATH on
        # sys/dev/qat/qat_api and then names
        # common/crypto/sym/lac_sym_api.c under it. Walk up to sys/.
        by_file, by_src, by_dir = kernel_flag_index(arch)
        # THIS file's own answer - the compile-with line that names it,
        # or the module whose SRCS does - goes in front, per kmod.mk:128.
        found: list[str] = list(by_file.get(rel, ())) + list(
            by_src.get(rel, ()))
        # A directory's is a guess about a file nothing named, and a
        # guess does not get to decide order: it stays at the end, where
        # it has always been. sys/modules/dtrace/dtnfscl takes .PATH on
        # sys/fs/nfsclient and builds ONE file from it; in front, its
        # ${OPENZFS_CFLAGS} put openzfs's SPL <sys/rwlock.h> ahead of
        # FreeBSD's and took nfs_clsubs.c and nfs_clkrpc.c from clean to
        # seventeen errors on rw_assert. Both halves were established by
        # compiling all 2,037 translation units whose flags this change
        # moves, twice.
        back: list[str] = []
        if not found:
            d = Path(rel).parent
            while str(d) not in (".", "sys"):
                back.extend(by_dir.get(str(d), ()))
                d = d.parent
        seen = set(flags)
        # Only the include flags move forward. The -D are order-free once
        # _dedupe_defines has keyed them by name, and leaving them where
        # they were keeps this change to the one thing kmod.mk:128 is
        # about.
        for f in found:
            if not f.startswith("-D") and f not in seen:
                seen.add(f)
                flags.append(f)
        # MAXUSERS is not an option anyone writes in a kernel config; it
        # is a NUMBER config(8) turns into one - mkoptions.cc:86 says
        # `/* Fake MAXUSERS as an option. */' - so it reaches the build
        # through a generated header this sweep does not synthesise, and
        # sys/kern/subr_param.c did not compile at all. That file sets
        # hz, maxfiles, nbuf and maxproc; `hz' being clamped there is an
        # argument this repository's own triage table leans on, made
        # about a file the analyser had never read.
        #
        # 0 is the auto-size value, which is what every kernel config in
        # this tree gets by not mentioning maxusers at all.
        # ...and the options this file's own `optional' clause names,
        # which config(8) would have written into an opt_*.h. First-wins
        # dedup means an explicit -D from a compile-with still beats it.
        flags += (list(opts) if opts is not None
                  else list(files_option_defines().get(rel, ())))
        flags += ["-DMAXUSERS=0"]
        flags += ["-D_KERNEL", "-DGENOFFSET",
                  "-D__has_c_attribute(x)=0",
                  # The kernel build force-includes this into every
                  # translation unit (sys/conf/kern.pre.mk), and it is
                  # where config(8) puts the options that are not tied to
                  # one subsystem - INET's neighbours, INVARIANTS, SMP.
                  "-include", "opt_global.h",
                  f"-I{opt_shim(arch)}",
                  # device_if.h and friends: generated, not shipped.
                  f"-I{iface_shim(arch)}",
                  # ...and the nineteen recipes in sys/conf/files, which
                  # are generated into the object directory the same way
                  # and reached through the build's own -I. on it.
                  f"-I{gen_headers()}",
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
                  # <dt-bindings/...> - the constants a device tree and
                  # the driver reading it agree on. sys/conf/files.arm64
                  # adds this with a compile-with for one file; every
                  # per-SoC clock and pinctrl driver wants it and the
                  # per-SoC files.* do not say so. Nothing else in the
                  # tree provides a dt-bindings/ directory, so it cannot
                  # shadow anything.
                  f"-I{SRC}/sys/contrib/device-tree/include",
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
        flags += defaults_options(arch)
        flags += arch_makefile_flags(arch)
        # Only a cpu name sys/conf/options* declares becomes a macro,
        # because that is the list config(8) looks the faked option up
        # in - the same rule files_option_defines() follows.
        _opts = _declared_options()
        _cpu = (cpu if cpu is not None
                else files_cpu_index().get(rel, {}).get(arch, ()))
        flags += [f"-D{_opts[c.lower()]}" for c in _cpu
                  if c.lower() in _opts]
        # ...the -D held back above, and then the directory's guess.
        seen = set(flags)
        for f in list(found) + back:
            if f not in seen:
                seen.add(f)
                flags.append(f)
        rd = resource_dir(cc)
        if rd:
            flags.append(f"-I{rd}")
        # The marker a module's .for rule leaves when it compiles with
        # ${CFLAGS:N-nostdinc}: that rule wants the standard headers, so
        # take -nostdinc back off. Ten files, all of them SIMD
        # implementations whose <immintrin.h> pulls clang's own
        # <mm_malloc.h>, which calls malloc() and free().
        if "-DPBSD_WANTS_STDINC" in flags:
            flags = [f for f in flags
                     if f not in ("-nostdinc", "-DPBSD_WANTS_STDINC")]
        return _dedupe_defines(flags)

    # The source's own directory first: many libc and msun sources include a
    # private header sitting beside them.
    flags.append(f"-I{(SRC / rel).parent}")

    # <rpcsvc/rquota.h> and its twenty siblings do not exist in the tree;
    # include/rpcsvc/Makefile says how to make them and rpc_headers()
    # runs it. Thirteen ERRORs in one shard were a missing one of these
    # and nothing else.
    flags.append(f"-I{rpc_headers()}")

    # ...and the headers the tree's other Makefiles install into
    # /usr/include, laid out the way they install them.
    flags.append(f"-I{incs_shim(arch)}")

    if rel.startswith("lib/libc"):
        flags += [f"-I{SRC}/lib/libc/include",
                  f"-I{SRC}/lib/libc/{LIBC_ARCH.get(arch, 'amd64')}",
                  f"-I{SRC}/lib/libc/resolv",
                  # softfloat/Makefile.inc puts the per-architecture
                  # directory FIRST and lists no third one:
                  #
                  #   CFLAGS+= -I${LIBC_SRCTOP}/${LIBC_ARCH}/softfloat \
                  #            -I${LIBC_SRCTOP}/softfloat
                  #
                  # softfloat/templates/ was on this path and had to
                  # come off it. It is upstream SoftFloat's TEMPLATE
                  # directory - files meant to be copied and edited -
                  # and its milieu.h opens with
                  #
                  #   #include "../../../processors/!!!processor.h"
                  #
                  # a deliberately unopenable path, so that using the
                  # template unedited fails loudly. It did: 39
                  # translation units, every one under lib/libc/arm/
                  # and the softfloat core, came back ERROR on it while
                  # the edited copy sat in lib/libc/arm/softfloat/ two
                  # entries below. A template read as though it were the
                  # program - the same mistake as hbsd_pax_SKEL.c.
                  f"-I{SRC}/lib/libc/{LIBC_ARCH.get(arch, 'amd64')}/softfloat",
                  f"-I{SRC}/lib/libc/softfloat",
                  f"-I{SRC}/lib/libc/softfloat/bits"
                  f"{'32' if LIBC_ARCH.get(arch) == 'arm' else '64'}",
                  # ...and the -D from the same three lines of that
                  # Makefile.inc. Without it arm-gcc.h defines neither
                  # FLOAT64_MANGLE nor FLOAT64_DEMANGLE - the whole pair
                  # is inside `#if defined(SOFTFLOAT_FOR_GCC)' - and
                  # every file that mangles a double fails to compile
                  # on an undeclared function. Harmless anywhere else:
                  # nothing outside softfloat reads it.
                  "-DSOFTFLOAT_FOR_GCC",
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
    # The flags the build really passes this component, read out of the
    # build. The walk below climbs from the source's directory to the
    # component root, which is right for lib/libc and blind to anything
    # that reaches sideways: libexec/atrun's Makefile is three lines and
    # one of them is `.include "${SRCTOP}/usr.bin/at/Makefile.inc"',
    # where nine of its -D live -- DAEMON_GID among them, without which
    # atrun.c does not compile. No walk up the tree finds that file.
    # bmake does, because following .include is what bmake is.
    #
    # Appended rather than substituted for the walk: where the two agree
    # the duplicate is dropped, and where the walk has something bmake
    # will not give (a directory whose Makefile bmake cannot read) the
    # walk still has it.
    comp = _component_dir(rel)
    if comp is not None:
        seen = set(flags)
        for f in userland_names.ask_cflags(comp, arch,
                                           name=Path(rel).name):
            if f not in seen:
                seen.add(f)
                flags.append(f)

    if rel.split("/")[0] in ("lib", "libexec"):
        # ...from the source's own directory up to the component root,
        # because a flag set in lib/libc/Makefile.inc applies to
        # lib/libc/gen/ too. Nearest first: a directory's own Makefile
        # is more specific than its parent's.
        d = (SRC / rel).parent
        root = SRC / rel.split("/")[0]
        base = Path(rel).name
        seen = set(flags)
        while True:
            ds, incs = makefile_flags(str(d), arch, base)
            for f in ds + incs:
                if f not in seen:
                    seen.add(f)
                    flags.append(f)
            if d == root or root not in d.parents:
                break
            d = d.parent
    if rel.startswith("lib/libc/csu/"):
        # The C start-up: libc_start1.c is the first C any process runs
        # after the run-time linker, and none of its six translation
        # units had ever compiled. lib/libc/csu/Makefile.inc is three
        # lines and they are all load-bearing:
        #
        #   .include "${LIBC_SRCTOP}/csu/${LIBC_ARCH}/Makefile.inc"
        #   CFLAGS+= -I${LIBC_SRCTOP}/csu/${LIBC_ARCH}
        #
        # and the per-architecture Makefile.inc is one line, a
        # -DCRT_IRELOC_{RELA,REL,SUPPRESS} that libc_start1.c turns into
        # `#error "Define platform reloc type"' when absent. Read out of
        # that Makefile.inc rather than copied into a table here, because
        # a table is a second place for the answer to be wrong in.
        csu = SRC / "lib/libc/csu" / LIBC_ARCH.get(arch, "amd64")
        flags.append(f"-I{csu}")
        mk = csu / "Makefile.inc"
        if mk.is_file():
            m = re.search(r"-D(CRT_IRELOC_\w+)", mk.read_text(errors="replace"))
            if m:
                flags.append(f"-D{m.group(1)}")
    if rel.startswith("libexec/rtld-elf"):
        # The run-time linker, which was not in any analyse shard until
        # the rtld.c aux_info defect cost thirty-seven boot runs -- and
        # which would still have reported nothing if it had been, because
        # `#include "notes.h"' did not resolve and the whole translation
        # unit came back ERROR. A file that will not compile reports zero
        # findings and is indistinguishable from a clean one.
        #
        # These are libexec/rtld-elf/Makefile's own flags:
        #   CFLAGS+= -I${SRCTOP}/lib/csu/common        (notes.h)
        #   CFLAGS+= -I${RTLD_ELF_DIR}/${RTLD_ARCH} -I${RTLD_ELF_DIR}
        #   CFLAGS+= -DIN_RTLD
        # RTLD_ARCH is MACHINE_ARCH where that subdirectory exists and
        # MACHINE_CPUARCH otherwise. LIBC_ARCH already carries that same
        # spelling for every architecture this sweep builds -- its values
        # (amd64, aarch64, arm, i386, powerpc64, riscv) are exactly the
        # subdirectory names under libexec/rtld-elf -- so it is reused
        # here rather than a second table being invented to drift from it.
        rtld = SRC / "libexec/rtld-elf"
        rtld_arch = arch if (rtld / arch).is_dir() \
            else LIBC_ARCH.get(arch, "amd64")
        flags += [f"-I{SRC}/lib/csu/common",
                  f"-I{rtld}/{rtld_arch}",
                  f"-I{rtld}",
                  f"-I{rtld}/rtld-libc",
                  "-DIN_RTLD"]
    if "/tests/" in rel:
        # A test program is not the library, but it is C in this tree and
        # it exercises the library's edge cases, which is where the
        # defects are. All 179 of them wanted <atf-c.h>, and it is in the
        # tree at contrib/atf - share/mk/atf.test.mk is what adds it to a
        # real build.
        flags.append(f"-I{SRC}/contrib/atf")

    flags += [f"-I{SRC}/lib/libsys", f"-I{SRC}/include", f"-I{SRC}/sys"]

    if rel.split("/")[0] in ("lib", "libexec", "usr.bin", "usr.sbin"):
        # Headers buildworld STAGES into /usr/include from somewhere
        # else in the tree, which a source tree has no equivalent of.
        # include/Makefile:39 takes .PATH on contrib/libc-vis for
        # <vis.h>, and lib/libutil/Makefile installs <libutil.h> by
        # INCS. Neither is reachable from where the source sits, so
        # xprintf_errno.c and the five posix1e files that want them do
        # not compile.
        #
        # Last on the path, so they fill a gap and never shadow a header
        # the source's own directory or its Makefile's -I would have
        # found. The general form of this is a staging directory built
        # from include/Makefile's own SRCS the way iface_shim() builds
        # the kernel's, and 90 lib/*/Makefile carry an INCS worth
        # staging; these two are the ones anything in scope asks for.
        flags += [f"-I{SRC}/contrib/libc-vis", f"-I{SRC}/lib/libutil"]
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
    # libsodium ships its test suite too. test/default/ is 72 standalone
    # programs with a main(), <stdio.h> and <assert.h>; sys/conf/files
    # names not one of them. "wants <signal.h> under -D_KERNEL" is not a
    # finding about the kernel.
    "contrib/libsodium/test/",
    # zstd ships its command-line tool and a zlib compatibility shim.
    # programs/ is zstdcli, benchzstd and fileio; zlibWrapper/ is gzread,
    # gzwrite and friends. sys/conf/files:644-664 names neither, and
    # usr.bin/zstd builds programs/ as userland.
    "contrib/zstd/programs/", "contrib/zstd/zlibWrapper/",
    # The CTF reader lives under sys/ and is built by
    # cddl/lib/libctf/Makefile - a userland library, for ctfconvert(1)
    # and ctfmerge(1). The kernel's CTF support is sys/kern/kern_ctf.c,
    # which is a different program. ctf_impl.h, the header these fail
    # on, is under src/cddl/ and not under src/sys/ at all.
    "cddl/contrib/opensolaris/uts/common/ctf/",
)


def is_kernel_tu(rel: str) -> bool:
    """False for the userland C that happens to live under sys/."""
    if not rel.startswith("sys/"):
        return True
    return not rel[len("sys/"):].startswith(NOT_KERNEL)


# The language standard is in the build system too, and this said c17
# where both halves of the tree say gnu17:
#
#   sys/conf/kern.mk:376      CSTD?=  gnu17
#   share/mk/bsd.sys.mk:13    CSTD?=  gnu17
#
# The difference is not academic. In strict ISO mode `typeof' is not a
# keyword, so linuxkpi's
#
#   #define kzalloc_obj(_p, ...)  kzalloc(sizeof(typeof(_p)), ...)
#
# is "error: expected expression" and every vendored Linux driver that
# allocates a struct fails to compile - twenty of iwlwifi's alone, on
# one macro. Statement expressions, case ranges and the bare `asm'
# spelling are the same story: the kernel is written in GNU C and was
# being read as ISO C.
STD_DEFAULT = {"kernel-c": "gnu17", "user-c": "gnu17",
               "kernel-c++": "c++23", "user-c++": "gnu++17"}
STD_LINE = re.compile(r"^(CSTD|CXXSTD)\s*\??=\s*(\S+)")


@functools.lru_cache(maxsize=None)
def _std(kind: str) -> str:
    """CSTD/CXXSTD as the makefile that governs this half of the tree
    sets it, falling back to what that file says today."""
    mk = (SYS / "conf" / "kern.mk") if kind.startswith("kernel") \
        else (SRC / "share" / "mk" / "bsd.sys.mk")
    want = "CXXSTD" if kind.endswith("c++") else "CSTD"
    if mk.is_file():
        # Depth 0 only, as kern_pre_vars() does: kern.mk carries a
        # `CSTD?= c89' inside a compiler conditional, and taking the
        # first line that matches picked that one.
        depth = 0
        for line in mk.read_text(errors="replace").splitlines():
            st = line.strip()
            if st.startswith((".if", ".for")):
                depth += 1
                continue
            if st.startswith((".endif", ".endfor")):
                depth = max(0, depth - 1)
                continue
            if depth:
                continue
            m = STD_LINE.match(st)
            if m and m.group(1) == want:
                return m.group(2)
    return STD_DEFAULT[kind]


def lang_flags(src: Path, rel: str | None = None,
               as_c: bool = False) -> list[str]:
    """`as_c' compiles a .cpp as C - see classify.py, where a landed port
    is a pure rename and CBMC's C front end is the stronger one."""
    try:
        rel = rel or src.resolve().relative_to(SRC).as_posix()
    except ValueError:
        rel = str(src)
    half = "kernel" if rel.startswith("sys/") else "user"
    if src.suffix == ".cpp" and not as_c:
        return ["-xc++", f"-std={_std(half + '-c++')}",
                "-fno-exceptions", "-fno-rtti"]
    return ["-xc", f"-std={_std(half + '-c')}"]


if __name__ == "__main__":
    import sys
    for a in sys.argv[1:]:
        p = Path(a)
        if not p.is_absolute():
            p = SRC / a
        print(" ".join(lang_flags(p) + include_flags(p)))
