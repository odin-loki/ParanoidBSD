#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Find code that several architectures each keep their own copy of.

lib/libc has the right shape: machine-dependent sources win, the
machine-independent C is the automatic fallback, and MK_MACHDEP_OPTIMIZATIONS
turns the whole machine-dependent layer off. sys/ has no equivalent - what is
under sys/<arch> is unconditional, and where two architectures need the same
logic they each carry a copy.

Those copies drift. sys/conf's HardenedBSD hardening was six copies and riscv's
was missing a security option. This looks for the same shape in code: files
with the same name under different architectures, scored by how similar they
are.

Similarity is line-set overlap after stripping comments and whitespace, which
is crude on purpose. It is a search for candidates, not a verdict: the point
is to rank where hoisting to sys/<something>/ shared code would pay, so a
person reads the top of the list rather than all of sys/.

sys/x86 already exists and is exactly this move made for amd64 and i386. The
question is what else deserves it.
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
    out = set()
    for line in text.splitlines():
        s = "".join(line.split())
        if s and not s.startswith("#include"):
            out.add(s)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--min-similarity", type=float, default=0.5)
    ap.add_argument("--min-lines", type=int, default=40)
    ap.add_argument("--top", type=int, default=25)
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    # basename -> arch -> path
    byname: dict[str, dict[str, str]] = collections.defaultdict(dict)
    for arch in ARCHES:
        base = os.path.join(root, "sys", arch)
        if not os.path.isdir(base):
            continue
        for dirpath, dirnames, filenames in os.walk(base):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            for name in filenames:
                if name.endswith((".c", ".h")):
                    byname[name].setdefault(arch, os.path.join(dirpath, name))

    rows = []
    for name, per_arch in byname.items():
        if len(per_arch) < 2:
            continue
        sets = {a: norm_lines(p) for a, p in per_arch.items()}
        sets = {a: s for a, s in sets.items() if len(s) >= args.min_lines}
        if len(sets) < 2:
            continue
        arches = sorted(sets)
        pairs = []
        for i, a in enumerate(arches):
            for b in arches[i + 1:]:
                sa, sb = sets[a], sets[b]
                inter = len(sa & sb)
                union = len(sa | sb)
                if union:
                    pairs.append((inter / union, inter, a, b))
        if not pairs:
            continue
        best = max(pairs)
        if best[0] < args.min_similarity:
            continue
        shared_everywhere = set.intersection(*sets.values())
        rows.append((best[0], len(shared_everywhere), name, arches, best))

    rows.sort(reverse=True)
    print(f"files with the same name under 2+ architectures: {len(byname)}")
    print(f"scoring >= {args.min_similarity} similarity and >= {args.min_lines} lines: "
          f"{len(rows)}\n")
    total_shared = 0
    for sim, shared_all, name, arches, best in rows[:args.top]:
        total_shared += shared_all
        print(f"{sim:5.2f}  {name:<26} {len(arches)} arches: {','.join(arches)}")
        print(f"        best pair {best[2]}/{best[3]}, {best[1]} identical lines; "
              f"{shared_all} common to all {len(arches)}")
    if rows:
        allshared = sum(r[1] for r in rows)
        print(f"\nlines identical across every architecture that has the file, "
              f"summed over all {len(rows)} candidates: {allshared}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
