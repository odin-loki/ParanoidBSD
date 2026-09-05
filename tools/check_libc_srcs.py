#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Check that every source lib/libc names actually exists.

lib/libc names its sources by hand across sixty-odd Makefile.inc files, and
resolves them through .PATH rather than by directory, so a name that no
longer exists is not visibly wrong anywhere. Two ways it goes bad here:

  * a source is deleted and its SRCS entry is not. Removing the 74
    machine-dependent .S files from lib/libc dropped the MDSRCS entries in
    the string directories and missed the stdlib ones, so
    amd64/stdlib/Makefile.inc still asked for div.S, ldiv.S and lldiv.S and
    i386/stdlib/Makefile.inc for div.S and ldiv.S, none of them on disk.

  * a source survives but the file it #includes does not.
    powerpc64/string kept bcopy_vsx.S, which is macro definitions ending in
    `#include "bcopy.S"`, after bcopy.S was deleted - and kept
    bcopy_resolver.c, whose ifunc chooses between __bcopy_vsx and the
    __bcopy that bcopy.S defined. tools/check_source_includes.py covers
    that half; this one covers the first.

Neither is visible in any build PBSD runs. MDSRCS is dropped whole when
MK_MACHDEP_OPTIMIZATIONS is no, which is what src.conf.pbsd asks for, so
these entries are inert until something asks for the assembly back - and
the boot-image workflow has a machdep_asm input that does exactly that.

Resolution follows how bmake finds them, which is the whole point of doing
it this way at all: lib/libc is ONE build, every Makefile.inc it includes
appends to ONE .PATH list, and a source named in any of them can live in
any directory on it. So the search list is every .PATH in lib/libc/Makefile
and in every Makefile.inc under it, expanded for LIBC_SRCTOP, SRCTOP and
LIBC_ARCH, plus each file's own directory - and lib/libsys, which is where
recvmmsg.c and sendmmsg.c actually are. A .PATH entry holding a variable
this does not know becomes a glob, so softfloat/bits${SOFTFLOAT_BITS}
contributes both bits32 and bits64.

Resolving per-file instead reported nineteen sources that are all perfectly
findable: ldexp.c and trivial-getcontextx.c in gen, machdep_ldis*.c in
gdtoa, softfloat.c in softfloat/bits{32,64}. Every one of those is a
directory some OTHER Makefile.inc put on the path. A check that cannot tell
those from the three real ones is not a check.

Entries built by a rule in the same file (generated sources such as rpc's
crypt_clnt.c) are skipped, and so is anything whose name still contains a
variable after expansion. LIBC_ARCH is expanded to each of the eight
architecture directories in turn, since one tree has to satisfy all of
them.
"""

from __future__ import annotations

import argparse
import glob
import os
import re
import sys

SRCS = re.compile(r"^(MDSRCS|MISRCS|SRCS)\s*\+?=\s*(.*)$")
PATH_LINE = re.compile(r"^\.PATH:\s*(.*)$")
SUFFIXES = (".c", ".cpp", ".S", ".s")

# LIBC_ARCH is MACHINE_CPUARCH except that powerpc64 keeps its own
# directory, so each per-architecture Makefile.inc is checked under the
# directory it actually lives in.
ARCH_DIRS = ("aarch64", "amd64", "arm", "i386", "powerpc", "powerpc64",
             "powerpcspe", "riscv")


def expand(path: str, root: str, libc: str, arch: str) -> list[str]:
    """Directories a .PATH entry can mean, as a list rather than one answer.

    A variable this does not know - SOFTFLOAT_BITS, say, whose two values
    give softfloat/bits32 and softfloat/bits64, both of which hold a
    softfloat.c - becomes a glob rather than a reason to give up. Dropping
    such an entry is how the first version came to report softfloat.c
    missing when there are two of it.
    """
    path = (path.replace("${LIBC_SRCTOP}", libc)
                .replace("${SRCTOP}", root)
                .replace("${LIBC_ARCH}", arch)
                .replace("${MACHINE_CPUARCH}", arch)
                .replace("${MACHINE_ARCH}", arch)
                .replace("${.CURDIR}", libc))
    if "$" not in path:
        return [path]
    globbed = re.sub(r"\$\{[^}]*\}", "*", path)
    return [d for d in glob.glob(globbed) if os.path.isdir(d)]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero when a source does not resolve")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    libc = os.path.join(root, "lib", "libc")
    if not os.path.isdir(libc):
        print(f"no lib/libc under {args.root}", file=sys.stderr)
        return 2

    # One .PATH list for the whole library, as bmake has. Collected first,
    # from lib/libc/Makefile and every Makefile.inc under it, with
    # ${LIBC_ARCH} expanded to each architecture in turn.
    search: set[str] = {libc, os.path.join(root, "lib", "libsys")}
    makefiles: list[str] = []
    for dirpath, dirnames, filenames in os.walk(libc):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for fn in filenames:
            if fn == "Makefile.inc" or fn == "Makefile":
                makefiles.append(os.path.join(dirpath, fn))
    for mk in makefiles:
        search.add(os.path.dirname(mk))
        text = re.sub(r"\\\n", " ",
                      open(mk, encoding="utf-8", errors="replace").read())
        for line in text.splitlines():
            m = PATH_LINE.match(line)
            if not m:
                continue
            for p in m.group(1).split():
                for arch in ARCH_DIRS:
                    for e in expand(p, root, libc, arch):
                        if os.path.isdir(e):
                            search.add(e)
    search_list = sorted(search)

    missing: list[tuple[str, str, str]] = []
    checked = 0

    for dirpath, dirnames, filenames in os.walk(libc):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        if "Makefile.inc" not in filenames:
            continue
        mk = os.path.join(dirpath, "Makefile.inc")
        rel = os.path.relpath(mk, root)
        # Which architecture's tree is this under, for ${LIBC_ARCH}?
        parts = os.path.relpath(dirpath, libc).split(os.sep)
        arch = parts[0] if parts and parts[0] in ARCH_DIRS else "amd64"

        text = re.sub(r"\\\n", " ", open(mk, encoding="utf-8",
                                         errors="replace").read())

        # Names a rule in this file builds - generated sources.
        built = set(re.findall(r"^([\w.$${}]+\.[cSs]|[\w.]+\.cpp)\s*:",
                               text, re.M))

        for line in text.splitlines():
            m = SRCS.match(line)
            if not m:
                continue
            for name in m.group(2).split():
                if not name.endswith(SUFFIXES) or "$" in name:
                    continue
                if name in built:
                    continue
                checked += 1
                if not any(os.path.exists(os.path.join(d, name))
                           for d in search_list):
                    missing.append((rel, m.group(1), name))

    print(f"lib/libc sources checked: {checked}")
    if not missing:
        print("every one resolves to a file on disk.")
        return 0

    print(f"\n{len(missing)} name a file that is not on disk:")
    for mk, kind, name in sorted(missing):
        print(f"  {mk}: {kind} {name}")
    print("\n  Either the file was deleted and its entry was not, or the")
    print("  entry is new and the file was never added.")
    return 1 if args.gate else 0


if __name__ == "__main__":
    sys.exit(main())
