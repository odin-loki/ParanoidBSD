#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which assembly can be deleted because the C is already there.

Removing assembly is for portability: every hand-written .S is something a
new architecture has to provide before it can boot, and most of it duplicates
a machine-independent C file that already exists.

libc says which ones, and says it in the build rather than by inspection.
lib/libc/Makefile:159:

    .if empty(MDSRCS) || ${MK_MACHDEP_OPTIMIZATIONS} == no
    SRCS+=  ${MISRCS}
    .else
    SRCS+=  ${MDSRCS}
    .for _src in ${MISRCS}
    .if ${MDSRCS:R:M${_src:R}} == ""
    SRCS+=  ${_src}

Machine-dependent sources win, and any machine-independent source with no
machine-dependent variant of the same basename is added. So an MDSRCS entry
`strcat.S` whose basename also appears in MISRCS as `strcat.c` is optional by
construction: MK_MACHDEP_OPTIMIZATIONS=no already selects the C, and upstream
supports that configuration.

This reports those, and separately reports the .S entries with no C
counterpart, which are the ones a removal would actually have to write.
"""

from __future__ import annotations

import argparse
import collections
import os
import re
import sys

ASSIGN = re.compile(r"^\s*(MDSRCS|MISRCS)\s*[+:?]?=\s*(.*)$")


def entries(path: str, want: str) -> list[str]:
    """Collect one variable's values, following backslash continuations."""
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        return []
    out: list[str] = []
    i = 0
    while i < len(lines):
        m = ASSIGN.match(lines[i])
        if not m:
            i += 1
            continue
        var, value = m.group(1), m.group(2)
        while value.endswith("\\") and i + 1 < len(lines):
            i += 1
            value = value[:-1] + " " + lines[i].strip()
        if var == want:
            for tok in value.split():
                if tok.startswith("#"):
                    break
                if "$" not in tok:
                    out.append(tok)
        i += 1
    return out


def collect(root: str, sub: str, want: str) -> dict[str, list[str]]:
    """basename stem -> the Makefile.inc files that name it."""
    found: dict[str, list[str]] = collections.defaultdict(list)
    for dirpath, dirnames, filenames in os.walk(os.path.join(root, sub)):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for name in filenames:
            if not name.startswith("Makefile"):
                continue
            path = os.path.join(dirpath, name)
            for entry in entries(path, want):
                found[entry].append(os.path.relpath(path, root))
    return found


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--list", action="store_true",
                    help="print each removable file, not just the counts")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isdir(os.path.join(root, "lib", "libc")):
        print(f"no lib/libc under {args.root}", file=sys.stderr)
        return 2

    md = collect(root, "lib/libc", "MDSRCS")
    mi = collect(root, "lib/libc", "MISRCS")
    mi_stems = {os.path.splitext(e)[0] for e in mi}

    covered: list[tuple[str, str]] = []
    uncovered: list[tuple[str, str]] = []
    for entry, wheres in sorted(md.items()):
        if not entry.endswith(".S"):
            continue
        stem = os.path.splitext(entry)[0]
        target = covered if stem in mi_stems else uncovered
        for where in wheres:
            target.append((where, entry))

    def lines_of(where: str, entry: str) -> int:
        path = os.path.join(root, os.path.dirname(where), entry)
        try:
            with open(path, "rb") as fh:
                return sum(1 for _ in fh)
        except OSError:
            return 0

    cov_lines = sum(lines_of(w, e) for w, e in covered)
    unc_lines = sum(lines_of(w, e) for w, e in uncovered)

    print("lib/libc machine-dependent assembly")
    print(f"  C counterpart already in MISRCS: {len(covered):>4} files, "
          f"{cov_lines:>6} lines")
    print(f"  no C counterpart:                {len(uncovered):>4} files, "
          f"{unc_lines:>6} lines")
    print()
    print("The first group is removable with no code written: "
          "MK_MACHDEP_OPTIMIZATIONS=no")
    print("already selects the C, and lib/libc/Makefile:159 is upstream's.")

    if args.list:
        print("\n-- removable --")
        for where, entry in covered:
            print(f"  {os.path.dirname(where)}/{entry}  ({lines_of(where, entry)} lines)")
        print("\n-- needs C written first --")
        for where, entry in uncovered:
            print(f"  {os.path.dirname(where)}/{entry}  ({lines_of(where, entry)} lines)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
