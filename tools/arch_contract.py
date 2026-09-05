#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""What a new architecture has to provide, derived from the ones that exist.

"Can PBSD run on <arch>?" currently gets answered by starting a build and
reading what breaks. That is how sys/i386 and include/i386 were found - one
build run each, an hour apart, each reporting one missing file.

The answer is already in the tree. Six architectures are present; a header or
source that every one of them provides is not optional, it is the interface.
Intersect them and the interface falls out. Files most-but-not-all provide are
the interesting middle: either a real optional, or a gap in the port that
provides them least.

Nothing here is hand-maintained. Add an architecture and the contract
re-derives; the point is that it stops being folklore.
"""

from __future__ import annotations

import argparse
import collections
import os
import sys

ARCHES = ["amd64", "arm64", "arm", "i386", "powerpc", "riscv"]


def listing(root: str, arch: str, sub: str) -> set[str]:
    d = os.path.join(root, "sys", arch, sub)
    if not os.path.isdir(d):
        return set()
    out = set()
    for dirpath, dirnames, filenames in os.walk(d):
        dirnames[:] = [x for x in dirnames if x != ".git"]
        for name in filenames:
            rel = os.path.relpath(os.path.join(dirpath, name), d)
            out.add(rel)
    return out


def report(root: str, sub: str, label: str, show: bool) -> None:
    have = {a: listing(root, a, sub if sub != "@self" else a) for a in ARCHES}
    present = {a: v for a, v in have.items() if v}
    if not present:
        return
    counts: collections.Counter[str] = collections.Counter()
    for files in present.values():
        counts.update(files)

    n = len(present)
    required = sorted(f for f, c in counts.items() if c == n)
    common = sorted(f for f, c in counts.items() if n // 2 < c < n)

    print(f"== {label}  ({n} architectures: {', '.join(sorted(present))})")
    print(f"   required (all {n} provide it):        {len(required)}")
    print(f"   partial  (most but not all):          {len(common)}")
    print(f"   arch-specific (fewer than half):      "
          f"{sum(1 for c in counts.values() if c <= n // 2)}")

    if common:
        print("   the gaps, and who is missing them:")
        for f in common[:12]:
            missing = sorted(a for a, v in present.items() if f not in v)
            print(f"     {f:<34} missing: {', '.join(missing)}")
        if len(common) > 12:
            print(f"     ... and {len(common) - 12} more")
    if show:
        print("   required:")
        for f in required:
            print(f"     {f}")
    print()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--show-required", action="store_true")
    args = ap.parse_args()
    root = os.path.abspath(args.root)
    if not os.path.isdir(os.path.join(root, "sys")):
        print(f"no sys/ under {args.root}", file=sys.stderr)
        return 2

    print("Per-architecture contract, derived from the tree\n")
    report(root, "include", "sys/<arch>/include", args.show_required)
    report(root, "@self", "sys/<arch>/<arch>", args.show_required)
    report(root, "conf", "sys/<arch>/conf", args.show_required)
    return 0


if __name__ == "__main__":
    sys.exit(main())
