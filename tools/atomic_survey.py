#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Measure the shape of the six hand-written atomic implementations.

This reports what can be counted from the source and refuses to report what
cannot. That distinction is the whole point of the file.

Countable: how much code there is, how much of it is inline assembly, and how
the operation x width x ordering space is spelled out.

NOT countable, and the reason there is no parity check here: arm64 generates
its atomics with token-pasting macros -

    _ATOMIC_OP_IMPL(8,  w, b, op, ...)
    _ATOMIC_OP_IMPL(32, w,  , op, ...)

so atomic_add_32 exists on arm64 and no regex over the header will find it. A
first version of this tool read the headers literally and reported 50 atomics
"used by machine-independent code but missing on some architecture". Most were
macro-generated and present. Answering that question needs the preprocessor,
with each architecture's real include path, which means running it on FreeBSD -
not here.

So the numbers below are about maintenance burden, which is what they can
honestly measure, and the parity question is left open rather than answered
wrongly.
"""

from __future__ import annotations

import argparse
import collections
import os
import re
import sys

ARCHES = ["amd64", "arm64", "arm", "i386", "powerpc", "riscv"]
DEFINE = re.compile(r'#\s*define\s+(atomic_[a-z0-9_]+)')
FUNCDEF = re.compile(
    r'^\s*(?:static\s+)?(?:__inline|inline)?\s*[a-z_0-9\*\s]*?\b(atomic_[a-z0-9_]+)\s*\(',
    re.M)
GENERATOR = re.compile(r'#\s*define\s+_*ATOMIC[A-Z_]*\(')
ORDERINGS = {"acq", "rel"}
WIDTHS = {"8", "16", "32", "64", "char", "short", "int", "long", "ptr"}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    args = ap.parse_args()
    root = os.path.abspath(args.root)

    total_lines = total_asm = 0
    literal: dict[str, set[str]] = {}
    generators: dict[str, int] = {}
    print(f"{'arch':<9} {'lines':>6} {'asm':>5} {'literal names':>14} {'generators':>11}")
    for a in ARCHES:
        p = os.path.join(root, "sys", a, "include", "atomic.h")
        if not os.path.isfile(p):
            continue
        with open(p, "r", encoding="utf-8", errors="replace") as fh:
            t = fh.read()
        lines = t.count("\n")
        asm = len(re.findall(r'__asm', t))
        names = set(DEFINE.findall(t)) | set(FUNCDEF.findall(t))
        gens = len(GENERATOR.findall(t))
        literal[a] = names
        generators[a] = gens
        total_lines += lines
        total_asm += asm
        print(f"{a:<9} {lines:>6} {asm:>5} {len(names):>14} {gens:>11}")

    print(f"{'total':<9} {total_lines:>6} {total_asm:>5}")

    union = set.union(*literal.values())
    ops: collections.Counter[str] = collections.Counter()
    orders: collections.Counter[str] = collections.Counter()
    widths: collections.Counter[str] = collections.Counter()
    for n in union:
        parts = n[len("atomic_"):].split("_")
        o = [x for x in parts if x in ORDERINGS]
        w = [x for x in parts if x in WIDTHS]
        ops["_".join(x for x in parts if x not in ORDERINGS and x not in WIDTHS)] += 1
        orders["_".join(o) if o else "relaxed"] += 1
        for x in w:
            widths[x] += 1

    print(f"\nnames written out literally, union over six architectures: {len(union)}")
    print(f"  operations : {len(ops)}")
    print(f"  widths     : {len(widths)}  {sorted(widths)}")
    print(f"  orderings  : {len(orders)}  {sorted(orders)}")
    print(f"  a complete cross-product would be {len(ops) * len(widths) * len(orders)}")
    print("\nThe gap between those two numbers is the maintenance burden: the")
    print("space is regular and it is being filled in by hand, unevenly, six")
    print("times. Which cells are actually filled cannot be read off the")
    print("source - see the module docstring.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
