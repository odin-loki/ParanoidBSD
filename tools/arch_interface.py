#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which per-architecture headers export the same interface?

tools/arch_duplication.py ranks by how similar the *text* is, so it finds
copy-paste. That is the wrong question for the best consolidation
candidates, and the tree contains the proof: six copies of atomic.h score
0.01 similarity - six texts with almost nothing in common - and implement
one interface. Similarity ranked the single largest opportunity in sys/
last.

The right question is about the contract, not the prose: does every
architecture export the same names? Where the answer is yes and the texts
differ, there are N implementations of one thing and a generic one can
replace them. Where the answer is no, the difference is real - pte.h and
pmap.h differ because the MMU differs, and no amount of C++ changes that.

  agreement = |names in every architecture| / |names in any|

WHAT THIS CANNOT SEE, stated up front because a previous tool in this tree
got it wrong and read as a security finding. arm64's atomic.h writes

    _ATOMIC_OP_IMPL(32, w,  , op, llsc_asm_op, lse_asm_op, pre, ...)

and the name atomic_add_32 never appears in the file. No regex finds it.
So a low agreement score on a header full of generator macros is a limit of
this tool, not a fact about the header, and the generators column says where
that applies. Agreement is a LOWER BOUND on how much the interfaces share.
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

ARCHES = ["amd64", "arm64", "arm", "i386", "powerpc", "riscv"]

COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
# #define NAME or #define NAME(
DEFINE = re.compile(r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)", re.M)
# A definition or declaration whose name sits at the start of a line, which
# is the style this tree uses for function bodies, plus the one-line
# declaration form.
FUNC_OWNLINE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*\(", re.M)
FUNC_INLINE = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{]*\)\s*[;{]",
    re.M)
TYPEDEF = re.compile(r"^\s*typedef\b[^;]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*;", re.M | re.S)
# A macro whose body invokes another macro with arguments is a generator.
GENERATOR = re.compile(
    r"^\s*(?:#\s*define\s+\S+\s+)?[A-Z_][A-Z0-9_]{3,}\s*\(\s*[0-9A-Za-z_]", re.M)

# Names that are structure, not interface.
BORING = re.compile(
    r"^(_|__)|_H_$|^(if|for|while|switch|return|sizeof|defined|typeof)$")


def names_of(text: str) -> set[str]:
    text = COMMENT.sub("", text)
    found: set[str] = set()
    for rx in (DEFINE, FUNC_OWNLINE, FUNC_INLINE, TYPEDEF):
        for m in rx.finditer(text):
            n = m.group(1)
            if not BORING.search(n):
                found.add(n)
    return found


def generators_of(text: str) -> int:
    text = COMMENT.sub("", text)
    n = 0
    for line in text.splitlines():
        s = line.strip()
        if s.startswith("#define"):
            continue
        if re.match(r"^[A-Z_][A-Z0-9_]{3,}\s*\(", s):
            n += 1
    return n


def norm_lines(text: str) -> set[str]:
    text = COMMENT.sub("", text)
    return {" ".join(l.split()) for l in text.splitlines()
            if l.strip() and not l.strip().startswith("#include")}


def similarity(texts: list[str]) -> float:
    sets = [norm_lines(t) for t in texts]
    inter = set.intersection(*sets)
    union = set.union(*sets)
    return len(inter) / len(union) if union else 1.0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--min-arches", type=int, default=3)
    ap.add_argument("--min-names", type=int, default=8)
    ap.add_argument("--top", type=int, default=25)
    args = ap.parse_args()

    root = Path(args.root)
    byname: dict[str, dict[str, Path]] = collections.defaultdict(dict)
    for arch in ARCHES:
        d = root / "sys" / arch / "include"
        if not d.is_dir():
            continue
        for p in d.rglob("*.h"):
            byname[p.name][arch] = p

    rows = []
    for name, paths in byname.items():
        if len(paths) < args.min_arches:
            continue
        texts = {}
        sets = {}
        gens = {}
        for arch, p in paths.items():
            t = p.read_text(errors="replace")
            texts[arch] = t
            sets[arch] = names_of(t)
            gens[arch] = generators_of(t)
        union = set.union(*sets.values())
        inter = set.intersection(*sets.values())
        if len(union) < args.min_names:
            continue
        agreement = len(inter) / len(union)
        sim = similarity(list(texts.values()))
        lines = sum(len(t.splitlines()) for t in texts.values())
        rows.append((agreement, sim, lines, name, len(paths),
                     len(inter), len(union), sum(gens.values())))

    # The prize is high agreement with low similarity: one contract, several
    # unrelated implementations.
    rows.sort(key=lambda r: (-(r[0] - r[1]), -r[2]))

    print(f"headers in {args.min_arches}+ architectures under "
          f"sys/<arch>/include, ranked by how much more the interfaces")
    print("agree than the texts do\n")
    print(f"{'agree':>6s} {'sim':>5s} {'gap':>6s} {'lines':>6s} {'n':>2s} "
          f"{'shared':>7s} {'total':>6s} {'gen':>4s}  header")
    for (agr, sim, lines, name, n, ni, nu, ng) in rows[:args.top]:
        print(f"{agr:6.2f} {sim:5.2f} {agr - sim:6.2f} {lines:6d} {n:2d} "
              f"{ni:7d} {nu:6d} {ng:4d}  {name}")

    print()
    print("agree  fraction of exported names present in every architecture")
    print("sim    line-set similarity, the arch_duplication.py measure")
    print("gap    agree - sim. A large gap is the atomic.h shape: the same")
    print("       contract implemented several unrelated ways, which is the")
    print("       strongest case for writing it once.")
    print("gen    macro invocations that generate names this tool cannot")
    print("       see. Where this is large, agree is an underestimate.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
