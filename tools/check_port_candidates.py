#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Which .c files in a scope can be ported by renaming them, and which cannot.

A port in this project is usually a rename: the .cpp is byte-identical to the
.c and the commit is `git mv` plus one SRCS line. Whether that is SAFE is a
property of the tree, not of the file, and the hundred-file lib/msun batch
learned the properties one expensive run at a time:

  * run 28 died nine minutes into buildworld on

        s_cos.cpp:48:10: fatal error: 'e_rem_pio2.c' file not found

    because five of the hundred were files other sources #include BY NAME.
    A file on the receiving end of an #include is not an independent
    translation unit and its extension is not free.

  * three of the hundred and three originally chosen were named in
    lib/libgcc_s/Makefile, which has its own SRCS and no CXXFLAGS.

  * the batch had to avoid ARCH_SRCS entries, because a rename there
    interacts with the .c/.cpp pairing rule and a failure could not be
    attributed to one or the other.

Each of those was found by hand, and the first was found by a fifty-minute
build. This applies all of them at once, to every candidate, in a second.

What it checks, per .c file in the scope:

  included      something #includes it by name. Fatal: renaming it breaks
                every includer.
  not-in-srcs   no Makefile under the scope names it. There is nothing to
                port - the file is not built here.
  conditional   its SRCS line sits inside an .if block, so it is built for
                some configurations and not others. A batch wants the
                unconditional ones, so a failure is about the port and not
                about which architecture ran.
  shadowed      another file of the same basename exists elsewhere in the
                scope, where a .PATH may prefer it. Renaming one of a pair
                changes which one bmake picks.
  outside       a Makefile OUTSIDE the scope names it - lib/libgcc_s is the
                real case - so the port needs that consumer handled too.

With --report (the oracle's pass_report.json, which is only produced on a
FreeBSD host) it also requires ir.equal, ir.abi_equal and edits == 0. Those
are the oracle's claims and cannot be checked here; without the report this
lists what the TREE permits and says so.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "hbsd" / "src"

INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)
SRCS_LINE = re.compile(r"^\s*[A-Z0-9_]*SRCS[A-Za-z0-9_.]*\s*\+?[:?]?=")
COND_OPEN = re.compile(r"^\s*\.\s*(if|ifdef|ifndef|for)\b")
COND_CLOSE = re.compile(r"^\s*\.\s*endif\b|^\s*\.\s*endfor\b")


def makefile_srcs(mk: Path):
    """(name, conditional) for every source a Makefile names.

    Conditional means the line sits inside an .if/.for. Continuations are
    joined first so a name on the fifth line of a list is still attributed
    to the line that started it.
    """
    try:
        text = mk.read_text(errors="replace")
    except OSError:
        return
    text = re.sub(r"\\\n", " ", text)
    depth = 0
    for line in text.splitlines():
        if COND_CLOSE.match(line):
            depth = max(0, depth - 1)
            continue
        if COND_OPEN.match(line):
            depth += 1
            continue
        if not SRCS_LINE.match(line):
            continue
        for tok in line.split("=", 1)[1].split():
            if tok.endswith((".c", ".cpp", ".S", ".s")) and "$" not in tok:
                yield os.path.basename(tok), depth > 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--scope", default="lib/msun",
                    help="directory under hbsd/src to screen")
    ap.add_argument("--report", help="the oracle's pass_report.json, to also "
                                     "require IR-equal, ABI-equal, zero-edit")
    ap.add_argument("--show-rejected", action="store_true")
    args = ap.parse_args()

    scope = SRC / args.scope
    if not scope.is_dir():
        print(f"no {args.scope} under hbsd/src", file=sys.stderr)
        return 2

    # Who is #included by name, anywhere in the tree.
    included: set[str] = set()
    for f in SRC.rglob("*"):
        if f.suffix not in (".c", ".cpp", ".h", ".S") or not f.is_file():
            continue
        try:
            for t in INCLUDE.findall(f.read_text(errors="replace")):
                included.add(os.path.basename(t))
        except OSError:
            pass

    # Who names each source, inside the scope and outside it.
    inside: dict[str, bool] = {}          # name -> conditional
    outside: dict[str, list[str]] = defaultdict(list)
    for mk in SRC.rglob("Makefile*"):
        if not mk.is_file():
            continue
        in_scope = scope in mk.parents or mk.parent == scope
        for name, cond in makefile_srcs(mk):
            if in_scope:
                # unconditional wins if the name appears both ways
                inside[name] = inside.get(name, True) and cond
            else:
                outside[name].append(str(mk.relative_to(SRC)))

    # Basenames appearing more than once in the scope.
    seen: dict[str, list[Path]] = defaultdict(list)
    for f in scope.rglob("*.c"):
        seen[f.name].append(f)

    ok, rejected = [], []
    for f in sorted(scope.rglob("*.c")):
        rel = f.relative_to(SRC).as_posix()
        why = []
        if f.name in included:
            why.append("included by name elsewhere")
        if f.name not in inside:
            why.append("no Makefile in the scope names it")
        elif inside[f.name]:
            why.append("its SRCS line is inside an .if/.for")
        if len(seen[f.name]) > 1:
            others = ", ".join(p.relative_to(SRC).as_posix()
                               for p in seen[f.name] if p != f)
            why.append(f"same basename as {others}")
        if f.name in outside:
            why.append("named by " + ", ".join(sorted(set(outside[f.name]))))
        (ok if not why else rejected).append((rel, why))

    verdict = "the tree permits"
    if args.report:
        rep = json.loads(Path(args.report).read_text())
        good = {r["source"] for r in rep.get("records", [])
                if (r.get("ir") or {}).get("equal")
                and (r.get("ir") or {}).get("abi_equal")
                and r.get("edits", 1) == 0}
        before = len(ok)
        held, dropped = [], []
        for rel, _ in ok:
            (held if rel in good else dropped).append(rel)
        ok = [(r, []) for r in held]
        rejected += [(r, ["not IR-equal, ABI-equal and zero-edit in the "
                          "report"]) for r in dropped]
        verdict = (f"the tree permits and the oracle verified "
                   f"({before} tree-side, {len(ok)} after the report)")

    print(f"scope {args.scope}: {len(ok)} of {len(ok) + len(rejected)} .c "
          f"files can be ported by rename - {verdict}\n")
    for rel, _ in ok:
        print(f"  {rel}")
    if args.show_rejected and rejected:
        print(f"\nrejected ({len(rejected)}):")
        for rel, why in rejected:
            print(f"  {rel}")
            for w in why:
                print(f"      {w}")
    elif rejected:
        print(f"\n{len(rejected)} rejected; --show-rejected to see why.")
    if not args.report:
        print("\nNote: no --report, so IR-equality, ABI-equality and the")
        print("zero-edit property are NOT checked. That is the oracle's")
        print("half and it only runs on FreeBSD. Intersect this with the")
        print("committable list the oracle prints before committing.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
