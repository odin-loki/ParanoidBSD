#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Check that no port has renamed a source another source #includes by name.

Some C sources are compiled twice: once on their own, and once textually,
because a second source does

    #include "e_rem_pio2.c"

to inline them. lib/msun does this in twenty-eight places. Such a file is not
an independent translation unit - its name is part of another file's source
text - so renaming it .c -> .cpp silently breaks every includer, and the
break does not show up in any of the checks a rename otherwise gets: the
file still exists, SRCS still resolves, no two objects collide, and the port
itself is byte-identical. It shows up minutes into buildworld as

    s_cos.cpp:48:10: fatal error: 'e_rem_pio2.c' file not found

which is what happened to the hundred-file lib/msun batch. Five of the
hundred were files that other sources include, and one of them stopped the
world.

What this GATES on is one thing only: an #include of "X.c" where X.c is
gone from the tree and X.cpp is there instead. That is a PBSD port and
nothing else - .c -> .cpp is the only rename this project performs - so the
rule has no judgement in it and cannot misfire on vendor code.

It also LISTS, without failing, every other #include of a source file that
does not resolve. There are twenty-three in the tree as imported and all of
them are fine: contrib/netbsd-tests reaching for NetBSD kernel sources that
are not here, crypto/krb5's ev.c naming four back ends of which the build
selects one, lib/csu's ignore_init.c, wg's selftest bodies. They are listed
because a port that breaks one would otherwise be invisible, and they are
not gated because none of them is a thing this repository controls.

Resolution is by basename across the whole tree rather than by reproducing
each Makefile's -I list. That direction cannot produce a false alarm - if
the name exists nowhere under the root it cannot be found by any -I - and
the failure this exists to catch is exactly a name that no longer exists.
"""

from __future__ import annotations

import argparse
import os
import re
import sys

# Suffixes whose files are translation units, and so whose names a port can
# change. A .h is excluded: headers are not ported and not renamed.
SOURCE_SUFFIXES = (".c", ".cpp", ".cc", ".cxx", ".S", ".s")

INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)

SCAN_SUFFIXES = SOURCE_SUFFIXES + (".h", ".hpp", ".inc")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--scope", action="append", default=[],
                    help="limit to these paths under root, e.g. lib/msun. "
                         "Repeatable. Default is the whole tree.")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero when an include does not resolve")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isdir(root):
        print(f"no tree at {args.root}", file=sys.stderr)
        return 2

    # basename -> True, over every file in the tree. Built once.
    have: set[str] = set()
    files: list[str] = []
    scopes = [os.path.join(root, s) for s in args.scope]
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for fn in filenames:
            have.add(fn)
            if not fn.endswith(SCAN_SUFFIXES):
                continue
            full = os.path.join(dirpath, fn)
            if scopes and not any(full.startswith(s) for s in scopes):
                continue
            files.append(full)

    broken: list[tuple[str, str, str | None]] = []
    checked = 0
    for full in files:
        try:
            text = open(full, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        for target in INCLUDE.findall(text):
            base = os.path.basename(target)
            if not base.endswith(SOURCE_SUFFIXES):
                continue
            checked += 1
            if base in have:
                continue
            # The one gating case: a .c that a PBSD port renamed to .cpp.
            ported = (base[:-2] + ".cpp") if base.endswith(".c") else None
            if ported not in have:
                ported = None
            broken.append((os.path.relpath(full, root), target, ported))

    ports = [b for b in broken if b[2]]
    others = [b for b in broken if not b[2]]

    print(f"source #includes checked: {checked}")
    if others:
        print(f"\n{len(others)} do not resolve and are not PBSD ports "
              f"(listed, not gated - see the module docstring):")
        for where, target, _ in sorted(others):
            print(f"  {where}: #include \"{target}\"")
    if not ports:
        print("\nno #include names a source a port has renamed.")
        return 0

    print(f"\n{len(ports)} #include(s) name a .c that a port renamed "
          f"to .cpp:")
    for where, target, renamed in sorted(ports):
        print(f"  {where}")
        print(f"    #include \"{target}\"  - gone; {renamed} is there instead")
    print("\n  Either revert those renames - a file another source includes")
    print("  by name is not an independent translation unit - or port every")
    print("  includer and edit the #include lines in the same commit.")
    return 1 if args.gate else 0


if __name__ == "__main__":
    sys.exit(main())
