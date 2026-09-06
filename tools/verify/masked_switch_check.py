#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A switch over a bit mask with fewer arms than the mask has states.

    int d;

    switch (p->p_flag2 & (P2_ASLR_ENABLE | P2_ASLR_DISABLE)) {
    case 0:                d = PROC_ASLR_NOFORCE;       break;
    case P2_ASLR_ENABLE:   d = PROC_ASLR_FORCE_ENABLE;  break;
    case P2_ASLR_DISABLE:  d = PROC_ASLR_FORCE_DISABLE; break;
    }
    ...
    *(int *)data = d;

Two bits is four values and there are three arms, so ENABLE|DISABLE
together falls through with `d` uninitialised - and `*(int *)data`
copies it to the caller of procctl(2). That is a kernel stack
disclosure rather than a wrong answer, and it is invisible to
-Wuninitialized (verified: clang says nothing about it).

That one was found by clang's analyser, and the analyser only sees
translation units it can compile and paths it explores - the same
limit that reported sys/netinet/igmp.c and not its byte-identical twin
sys/netinet6/mld6.c. So this looks for the SHAPE, everywhere, which is
the lesson that keeps repeating: read the class, then grep for it.

WHAT IT REPORTS

A switch is flagged when all four hold:

  1. its controlling expression is `... & (A | B | ...)`, N names;
  2. it has no `default:`;
  3. it has fewer than 2**N `case` arms;
  4. at least one variable assigned in an arm was declared in the
     enclosing function without an initialiser.

(4) is what separates "the author enumerated the states that matter"
from "a value escapes uninitialised". A switch with no gap-reachable
uninitialised variable is not a defect, and there are many: an arm may
only `break`, or every arm may assign something already initialised.

WHAT IT CANNOT SEE, AND WHY --gate IS ABOUT THE NEXT ONE

  * A mask whose names are not all distinct bits. `(A | B)` where B is
    two bits has more than four states, so 2**N understates the gap
    and this UNDER-reports. It cannot expand macros.
  * A mask of an enum-like field where the combination is excluded by
    an invariant held elsewhere - which is true of both procctl sites
    and is why they were fixed rather than declared bugs. Reachability
    is a reading job, not a grep job.
  * A `default:` inside a nested switch is counted for the outer one.

The tree is at zero with the two procctl sites fixed, so --gate catches
a NEW one; it is not a claim that every hit is a bug. Read each one.
"""

from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"

# switch (<anything> & ( A | B [| C ...] ))
SWITCH = re.compile(r"switch\s*\(.*?&\s*\(\s*([A-Za-z_]\w*(?:\s*\|\s*[A-Za-z_]\w*)+)\s*\)")
CASE = re.compile(r"^\s*case\s")
DEFAULT = re.compile(r"^\s*default\s*:")
# `int d;` / `uint32_t x, y;` - a declaration with no `=`
DECL = re.compile(r"^\s*(?:const\s+)?(?:unsigned\s+|signed\s+|struct\s+|enum\s+)?"
                  r"[A-Za-z_]\w*(?:\s*\*)*\s+([A-Za-z_]\w*(?:\s*,\s*[A-Za-z_]\w*)*)\s*;\s*$")
ASSIGN = re.compile(r"^\s*([A-Za-z_]\w*)\s*(?:=[^=]|\|=|&=|\^=|\+=|-=)")


def uninitialised_locals(lines: list[str], switch_ln: int) -> set[str]:
    """Names declared without an initialiser above SWITCH_LN, back to `{`.

    Walking back to the opening brace of the function is approximate and
    deliberately so: it costs a few false names, and a false name only
    matters if that name is also assigned inside the switch.
    """
    names: set[str] = set()
    depth = 0
    for i in range(switch_ln - 1, max(-1, switch_ln - 200), -1):
        line = lines[i]
        depth += line.count("}") - line.count("{")
        if depth < 0:                       # left the function body
            break
        m = DECL.match(line)
        if m:
            names |= {n.strip() for n in m.group(1).split(",")}
    return names


def switch_body(lines: list[str], start: int) -> tuple[list[str], int]:
    """Lines of the switch body, and the index one past its closing brace."""
    depth, body, seen = 0, [], False
    for i in range(start, min(len(lines), start + 400)):
        line = lines[i]
        depth += line.count("{")
        if depth:
            seen = True
            body.append(line)
        depth -= line.count("}")
        if seen and depth <= 0:
            return body, i + 1
    return body, min(len(lines), start + 400)


def sites(path: Path):
    try:
        text = path.read_text(errors="replace")
    except OSError:
        return
    # Comments carry example code - the fix for the two procctl sites
    # quotes the very switch it fixed, and nowait_check.py read its own
    # explanatory comment as a bug before it learned this. Blank the
    # bodies rather than delete them: deleting shifts every line number
    # below, and a checker that names the wrong line is worse than one
    # that names none. (Verified: it reported :724 for a defect on :820.)
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"),
                  text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    lines = text.splitlines()
    for ln, line in enumerate(lines):
        m = SWITCH.search(line)
        if not m:
            continue
        names = [n.strip() for n in m.group(1).split("|")]
        body, _ = switch_body(lines, ln)
        if any(DEFAULT.match(b) for b in body):
            continue
        cases = sum(1 for b in body if CASE.match(b))
        if cases >= 2 ** len(names):
            continue
        decls = uninitialised_locals(lines, ln)
        assigned = {a.group(1) for b in body if (a := ASSIGN.match(b))}
        escaping = sorted(decls & assigned)
        if escaping:
            yield ln + 1, names, cases, escaping, line.strip()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--scope", action="append",
                    help="tree-relative path; repeatable (default sys)")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on any finding. The tree is at zero; a "
                         "false positive is fixed by teaching sites() a "
                         "new class, not by an allowlist.")
    args = ap.parse_args()

    hits = []
    for scope in (args.scope or ["sys"]):
        base = SRC / scope
        files = ([base] if base.is_file()
                 else sorted(base.rglob("*.c")) if base.is_dir() else [])
        for f in files:
            for ln, names, cases, escaping, src in sites(f):
                hits.append((f.relative_to(SRC).as_posix(), ln, names,
                             cases, escaping, src))

    print(f"{len(hits)} masked switch(es) with an uninitialised escape\n")
    by = collections.Counter(h[0].rsplit("/", 1)[0] for h in hits)
    for d, n in by.most_common():
        print(f"  {n:3d}  {d}")
    if hits:
        print()
    for rel, ln, names, cases, escaping, src in hits:
        print(f"{rel}:{ln}  ({', '.join(escaping)})")
        print(f"    {src[:78]}")
        print(f"    -> {len(names)} bits = {2 ** len(names)} states, "
              f"{cases} case arm(s), no default")
    if hits:
        print("\nRead each one. A combination excluded by an invariant held")
        print("in another file is not a bug - it is a fix waiting for one")
        print("new `|=` somewhere. What this cannot see is in the docstring.")
    if args.gate and hits:
        print(f"\nFAIL  {len(hits)} masked switch(es) leave a variable")
        print("      uninitialised. The tree was at zero when this was added.")
        return 1
    if args.gate:
        print("no masked switch leaves a variable uninitialised")
    return 0


if __name__ == "__main__":
    sys.exit(main())
