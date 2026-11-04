#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Six copies of a file are often two implementations. Which two?

arch_duplication.py reports the best pair, which answers "is there
copy-paste here" and not "how many different things are actually written".
The difference matters, because it is the difference between one edit and
several.

sys/<arch>/<arch>/uio_machdep.c is the case that made this worth writing.
Six files, 938 lines, and the similarity table ranks it second in the whole
of sys/ at 0.97 - which reads as "six near-identical copies, hoist it". It
is not. There are exactly two implementations and the split is real:

  pmap_map_io_transient   amd64, arm64, riscv    architectures with a direct map
  sf_buf_alloc            arm, i386, powerpc     architectures without one

Hoisting that to one file would have meant picking one and breaking three.
The right change is two machine-independent implementations selected by a
property the architectures already have, and you cannot see that from a
pairwise maximum.

So this clusters. Single linkage over the similarity graph at a threshold:
two files are in the same cluster when some chain of >= threshold pairs
joins them. The output is "n copies, k implementations", ranked by how many
lines the k-th implementation would save.
"""
from __future__ import annotations

import argparse
import collections
import os
import re
import sys

ARCHES = ["amd64", "arm64", "arm", "i386", "powerpc", "riscv"]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def norm_lines(path: str) -> set[str]:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            text = fh.read()
    except OSError:
        return set()
    text = COMMENT.sub("", text)
    return {"".join(l.split()) for l in text.splitlines()
            if l.strip() and not l.strip().startswith("#include")}


def line_count(path: str) -> int:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            return sum(1 for _ in fh)
    except OSError:
        return 0


def cluster(arches: list[str], sets: dict[str, set[str]],
            threshold: float) -> list[list[str]]:
    """Single linkage. Deliberately generous: it takes a chain of similar
    pairs as one implementation, so the count of clusters is a LOWER bound
    on how many distinct things there are. Undercounting is the safe
    direction here - it makes the tool claim less consolidation is possible,
    not more."""
    parent = {a: a for a in arches}

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    for i, a in enumerate(arches):
        for b in arches[i + 1:]:
            sa, sb = sets[a], sets[b]
            u = len(sa | sb)
            if u and len(sa & sb) / u >= threshold:
                ra, rb = find(a), find(b)
                if ra != rb:
                    parent[ra] = rb
    groups: dict[str, list[str]] = collections.defaultdict(list)
    for a in arches:
        groups[find(a)].append(a)
    return sorted(groups.values(), key=lambda g: (-len(g), g[0]))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--scope", default="sys")
    ap.add_argument("--threshold", type=float, default=0.80)
    ap.add_argument("--min-arches", type=int, default=3)
    ap.add_argument("--min-lines", type=int, default=40)
    ap.add_argument("--top", type=int, default=20)
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    byname: dict[str, dict[str, str]] = collections.defaultdict(dict)
    for arch in ARCHES:
        base = os.path.join(root, args.scope, arch)
        if not os.path.isdir(base):
            continue
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            for name in filenames:
                if name.endswith((".c", ".h")):
                    byname[name].setdefault(arch, os.path.join(dirpath, name))

    rows = []
    for name, per_arch in byname.items():
        if len(per_arch) < args.min_arches:
            continue
        sets = {a: norm_lines(p) for a, p in per_arch.items()}
        sets = {a: s for a, s in sets.items() if len(s) >= args.min_lines}
        if len(sets) < args.min_arches:
            continue
        arches = sorted(sets)
        groups = cluster(arches, sets, args.threshold)
        lines = sum(line_count(per_arch[a]) for a in arches)
        n, k = len(arches), len(groups)
        # What one implementation per cluster would leave behind.
        saved = int(lines * (n - k) / n)
        rows.append((saved, lines, n, k, name, groups))

    rows.sort(reverse=True)
    total = sum(r[0] for r in rows)
    print(f"files under {args.scope}/<arch>/ in {args.min_arches}+ "
          f"architectures, clustered at similarity >= {args.threshold}\n")
    print(f"{'saves':>6s} {'lines':>6s} {'n':>2s} {'k':>2s}  file")
    for saved, lines, n, k, name, groups in rows[:args.top]:
        print(f"{saved:6d} {lines:6d} {n:2d} {k:2d}  {name}")
        for g in groups:
            print(f"{'':22s}{','.join(g)}")
    print()
    print(f"n  copies    k  distinct implementations at this threshold")
    print(f"saves  lines that one implementation per cluster would remove")
    print(f"\nover the whole list: {total} lines, in {len(rows)} files")
    print("k > 1 is the warning. A file with six copies and two")
    print("implementations does not get hoisted to one - it gets two")
    print("machine-independent implementations and a way to choose, and")
    print("choosing wrong breaks the architectures in the other cluster.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
