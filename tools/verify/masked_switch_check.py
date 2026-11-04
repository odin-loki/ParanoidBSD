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

  1. its controlling expression masks off a small field, either as
     `... & (A | B | ...)` with N names or as a literal `... & 0x00f0`
     of at most four bits - a shift around it changes nothing;
  2. it has no `default:`;
  3. it has fewer `case` arms than the mask has states, 2**popcount;
  4. at least one variable assigned in an arm was declared in the
     enclosing function without an initialiser AND is still read after
     the switch closes.

(4) is what separates "the author enumerated the states that matter"
from "a value escapes uninitialised". A switch with no gap-reachable
uninitialised variable is not a defect, and there are many: an arm may
only `break`, or every arm may assign something already initialised.
The second half of (4) is the rest of it - a name the arm sets and the
arm alone reads never leaves the arm, whatever the gap. Both halves
were needed: without the read test nfscl_fillsattr() reports, with it
the tree is at zero and the procctl shape still does.

WHAT IT CANNOT SEE, AND WHY --gate IS ABOUT THE NEXT ONE

  * A mask whose names are not all distinct bits. `(A | B)` where B is
    two bits has more than four states, so 2**N understates the gap
    and this UNDER-reports. It cannot expand macros. (A literal mask
    has no such doubt - popcount is exact - which is the one way the
    literal form is the stronger of the two.)
  * A literal mask of more than four bits, deliberately: see
    mask_states(). Over the seven scopes it walks (10,911 files) it
    accepts 178 masked switches - 60 named, 118 literal - of which 38
    have a state gap; 90 more literal masks are cut by that line, and
    every one that was read is an opcode decoder.
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

# The same set capacity_first.py walks. The rule used to default to sys
# alone, from when it was a kernel-stack-disclosure rule; it costs five
# seconds over all of it, so the gate should not be looking at a seventh
# of the tree.
SCOPES = ("lib", "bin", "sbin", "usr.bin", "usr.sbin", "libexec", "sys")

SWITCH_KW = re.compile(r"\bswitch\s*\(")
# ... & ( A | B [| C ...] ) - a mask spelled as named bits
MASK_NAMES = re.compile(r"(?<!&)&(?!&)\s*\(\s*"
                        r"([A-Za-z_]\w*(?:\s*\|\s*[A-Za-z_]\w*)+)\s*\)")
# ... & 0x00f0 - the same mask spelled as a literal
MASK_LIT = re.compile(r"(?<!&)&(?!&)\s*\(?\s*(0[xX][0-9a-fA-F]+|\d+)[uUlL]*\s*\)?")
CASE = re.compile(r"^\s*case\s")
DEFAULT = re.compile(r"^\s*default\s*:")
# `int d;` / `uint32_t x, y;` - a declaration with no `=`
DECL = re.compile(r"^\s*(?:const\s+|volatile\s+)*"
                  r"(?:unsigned\s+|signed\s+|struct\s+|enum\s+)?"
                  r"[A-Za-z_]\w*\s*"
                  r"((?:\*\s*)*[A-Za-z_]\w*"
                  r"(?:\s*,\s*(?:\*\s*)*[A-Za-z_]\w*)*)\s*;\s*$")
ASSIGN = re.compile(r"^\s*([A-Za-z_]\w*)\s*(?:=[^=]|\|=|&=|\^=|\+=|-=)")
# `case P2_ASLR_ENABLE:   d = PROC_ASLR_FORCE_ENABLE;  break;` - the arm
# and its assignment on one line is the house style for a short switch,
# and it is the shape in this file's own docstring. Strip the label
# before ASSIGN, or the commonest spelling of condition (4) is invisible.
LABEL = re.compile(r"^\s*(?:case\s[^:]*|default\s*):\s*")


def scrutinee(lines: list[str], ln: int, open_paren: int) -> str:
    """The controlling expression of the switch, parens balanced.

    Line-based like the rest of this checker, but a controlling
    expression is allowed to wrap - `switch ((sc->sc_flags &\n    (F_A |
    F_B)))` is one the tree actually contains - so read forward until the
    `switch (` closes, at most a handful of lines.
    """
    depth, out = 0, []
    for i in range(ln, min(len(lines), ln + 6)):
        line = lines[i][open_paren if i == ln else 0:]
        for j, ch in enumerate(line):
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    out.append(line[:j])
                    return "".join(out)
        out.append(line)
    return "".join(out)


def c_int(lit: str) -> int:
    """A C integer literal. `0377` is octal 255; int(x, 0) raises on it."""
    if lit[:2] in ("0x", "0X"):
        return int(lit, 16)
    if len(lit) > 1 and lit[0] == "0":
        return int(lit, 8)
    return int(lit)


def mask_states(expr: str):
    """How many values the masked scrutinee can take, and what to call it.

    The count is 2**popcount(mask) whichever way the mask is written, and
    a shift does not change it: `(mca_error & 0x000c) >> 2` has the same
    four states as `flags & (A | B)`. So a literal mask is the same rule,
    not a new one - the only difference is that popcount is exact for a
    literal and a lower bound for names (a name may cover two bits).

    A named mask is preferred when both are present, because the names
    are what a reader of the report needs to see.
    """
    m = MASK_NAMES.search(expr)
    if m:
        names = [n.strip() for n in m.group(1).split("|")]
        return 2 ** len(names), " | ".join(names)
    m = MASK_LIT.search(expr)
    if m:
        bits = bin(c_int(m.group(1))).count("1")
        # Past a nibble the switch is a decoder over an encoding, not a
        # read of a small flag field, and 2**popcount stops being the
        # number of REACHABLE states: `instr & 0x7f` is 128 by that
        # formula and 10 in fact, `cqe->opcode & 0x1f` is 32 and a dozen.
        # Every site in the tree above four bits is of that kind
        # (checked: 0x1f, 0x3f, 0x7f, 0xff, 0xff0, 0xff000000), so the
        # cut costs nothing today and keeps --gate meaningful tomorrow.
        if not 1 <= bits <= 4:
            return None
        return 2 ** bits, m.group(1)
    return None


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
            names |= {n.strip().lstrip("*").strip()
                      for n in m.group(1).split(",")}
    return names


def read_after(lines: list[str], end: int, names: set[str]) -> set[str]:
    """Of NAMES, those the enclosing function still mentions past END.

    This is what makes the shape a defect rather than a style. `d` in
    the procctl switch is read by `*(int *)data = d;` after it, and a
    kernel stack disclosure follows. `np` in nfscl_fillsattr() is set at
    the top of the ND_NFSV4 arm and read only inside it, so the three
    arms for eight states cost nothing at all - and without this test it
    reads exactly like the procctl one.

    Under-reports a switch inside a loop whose variable is read at the
    top of the NEXT iteration; over-reports a mention inside a later
    `else` that the gap path cannot reach. Both are a reading job.
    """
    tail = []
    for i in range(end, min(len(lines), end + 400)):
        if lines[i].startswith("}"):            # end of the function
            break
        tail.append(lines[i])
    body = "\n".join(tail)
    return {n for n in names
            if re.search(r"\b%s\b" % re.escape(n), body)}


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
        m = SWITCH_KW.search(line)
        if not m:
            continue
        st = mask_states(scrutinee(lines, ln, m.end() - 1))
        if st is None:
            continue
        states, label = st
        body, end = switch_body(lines, ln)
        if any(DEFAULT.match(b) for b in body):
            continue
        cases = sum(1 for b in body if CASE.match(b))
        if cases >= states:
            continue
        decls = uninitialised_locals(lines, ln)
        assigned = {a.group(1) for b in body
                    if (a := ASSIGN.match(LABEL.sub("", b)))}
        escaping = sorted(read_after(lines, end, decls & assigned))
        if escaping:
            yield ln + 1, label, states, cases, escaping, line.strip()


def format_hit(rel: str, ln: int, label: str, states: int, cases: int,
               escaping: list[str], src: str) -> list[str]:
    """The three lines a finding prints.

    A function of its own because the tree is at zero, so nothing else
    ever runs this code - and it had a `len(names)` in it, left from the
    named-mask-only version, that would have raised NameError on the
    first real finding this rule ever made. A rule that reports zero
    hides its own reporting bugs; test_masked_switch_check.py now calls
    this directly.
    """
    return [f"{rel}:{ln}  ({', '.join(escaping)})",
            f"    {src[:78]}",
            f"    -> {label}: {states} states, {cases} case arm(s), "
            f"no default"]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--scope", action="append",
                    help="tree-relative path; repeatable (default: the "
                         "PBSD-owned scopes, which take five seconds)")
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 on any finding. The tree is at zero; a "
                         "false positive is fixed by teaching sites() a "
                         "new class, not by an allowlist.")
    args = ap.parse_args()

    hits = []
    for scope in (args.scope or SCOPES):
        base = SRC / scope
        files = ([base] if base.is_file()
                 else sorted(base.rglob("*.c")) if base.is_dir() else [])
        for f in files:
            for ln, label, states, cases, escaping, src in sites(f):
                hits.append((f.relative_to(SRC).as_posix(), ln, label,
                             states, cases, escaping, src))

    print(f"{len(hits)} masked switch(es) with an uninitialised escape\n")
    by = collections.Counter(h[0].rsplit("/", 1)[0] for h in hits)
    for d, n in by.most_common():
        print(f"  {n:3d}  {d}")
    if hits:
        print()
    for hit in hits:
        print("\n".join(format_hit(*hit)))
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
