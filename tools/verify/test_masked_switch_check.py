#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""masked_switch_check.py reports ZERO, and zero is what needs a test.

A rule that finds nothing looks exactly like a rule that has stopped
looking, and this one now covers two spellings of the same mask -- the
named `flags & (A | B)` it was written for, and the literal `x & 0x000c`
that turned out to be the commoner form in the tree.  Nothing in the
output distinguishes "no defect" from "the literal pattern never
matched", so these checks do.

The last one is the sharpest.  The two procctl switches this rule was
written for are still in the tree, still with three arms for four
states and still without a `default:` -- they clear the rule only at
condition (4), because the fix initialised the variable.  Assert that,
and a regression that removes the initialiser cannot pass silently.
"""
from __future__ import annotations

import re
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
ROOT = HERE.parents[1]
SRC = ROOT / "hbsd" / "src"

from masked_switch_check import (             # noqa: E402
    ASSIGN, CASE, DEFAULT, LABEL, SWITCH_KW, c_int, format_hit,
    mask_states, read_after, scrutinee, sites, switch_body,
    uninitialised_locals)

fails: list[str] = []


def check(what: str, ok: bool, why: str = "") -> None:
    print(f"  {'ok  ' if ok else 'FAIL'}   {what}"
          + (f"  -- {why}" if not ok and why else ""))
    if not ok:
        fails.append(what)


def hits(body: str) -> list:
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "t.c"
        p.write_text(body)
        return list(sites(p))


def wrap(sw: str, decl: str = "\tint d;\n") -> str:
    return "void f(unsigned x)\n{\n" + decl + sw + "\n\tg(d);\n}\n"


print("the shape it is FOR -- a named mask")

check("three arms for four states, d escapes",
      len(hits(wrap("""\
\tswitch (x & (A | B)) {
\tcase 0:\t\td = 1; break;
\tcase A:\t\td = 2; break;
\tcase B:\t\td = 3; break;
\t}"""))) == 1)

check("all four arms present",
      hits(wrap("""\
\tswitch (x & (A | B)) {
\tcase 0:\t\t d = 1; break;
\tcase A:\t\t d = 2; break;
\tcase B:\t\t d = 3; break;
\tcase A | B:\t d = 4; break;
\t}""")) == [])

check("a default: closes the gap",
      hits(wrap("""\
\tswitch (x & (A | B)) {
\tcase 0:\t\td = 1; break;
\tdefault:\td = 2; break;
\t}""")) == [])

check("nothing uninitialised escapes",
      hits(wrap("""\
\tswitch (x & (A | B)) {
\tcase 0:\t\td = 1; break;
\tcase A:\t\td = 2; break;
\t}""", decl="\tint d = 0;\n")) == [])


check("a name the arm sets and the arm alone reads does not escape",
      hits("void f(unsigned x)\n{\n\tint d;\n"
           "\tswitch (x & (A | B)) {\n"
           "\tcase 0:\t\td = 1; g(d); break;\n"
           "\tcase A:\t\td = 2; g(d); break;\n"
           "\t}\n}\n") == [])

check("a pointer declared without an initialiser is seen",
      len(hits("void f(unsigned x)\n{\n\tstruct nfsnode *np;\n"
               "\tswitch (x & (A | B)) {\n"
               "\tcase 0:\t\tnp = a(); break;\n"
               "\tcase A:\t\tnp = b(); break;\n"
               "\t}\n\tg(np);\n}\n")) == 1)

check("and so is the second name of `int *p, *q;`",
      len(hits("void f(unsigned x)\n{\n\tint *p, *q;\n"
               "\tswitch (x & (A | B)) {\n"
               "\tcase 0:\t\tq = a(); break;\n"
               "\tcase A:\t\tq = b(); break;\n"
               "\t}\n\tg(q);\n}\n")) == 1)


print("the same mask written as a literal")

check("x & 0xc, two arms of four",
      len(hits(wrap("""\
\tswitch (x & 0xc) {
\tcase 0x0:\td = 1; break;
\tcase 0x4:\td = 2; break;
\t}"""))) == 1)

check("masked then shifted, (x & 0x00f0) >> 4",
      len(hits(wrap("""\
\tswitch ((x & 0x00f0) >> 4) {
\tcase 0:\t\td = 1; break;
\tcase 1:\t\td = 2; break;
\tcase 2:\t\td = 3; break;
\t}"""))) == 1)

check("shifted then masked, (x >> 28) & 0xf",
      len(hits(wrap("""\
\tswitch ((x >> 28) & 0xf) {
\tcase 0:\t\td = 1; break;
\tcase 1:\t\td = 2; break;
\t}"""))) == 1)

check("the shift does not change the state count",
      mask_states("(x & 0x000c) >> 2") == mask_states("x & 0x000c")
      == (4, "0x000c"))

check("a named mask is preferred when both are present",
      mask_states("(x & (A | B)) & 0xf")[1] == "A | B")


print("where the literal form stops -- past a nibble it is a decoder")

check("instr & 0x7f is not read as 128 states",
      mask_states("instr & 0x7f") is None)
check("cqe->opcode & 0x1f is not read as 32 states",
      mask_states("cqe->opcode & 0x1f") is None)
check("a nibble is still read", mask_states("x & 0xf") == (16, "0xf"))
check("one bit is still read", mask_states("x & 1") == (2, "1"))
check("a zero mask is not a state space", mask_states("x & 0") is None)


print("literals the tree actually contains")

check("0377 is octal 255, not decimal", c_int("0377") == 255)
check("0xff0 parses", c_int("0xff0") == 0xff0)
check("3 is three", c_int("3") == 3)
check("the octal form does not crash the rule",
      mask_states("x & 0377") is None)


print("what must NOT be read as a mask")

check("&& is not a bitwise and", mask_states("a && 0xf") is None)
check("address-of is not a mask", mask_states("f(&x, 3)") is None)
check("a bare switch has no mask", mask_states("sc->state") is None)
check("a shift with no mask is not counted",
      mask_states("cqe->op_own >> 4") is None)


print("the scrutinee, which is allowed to wrap")

lines = ["\tswitch ((sc->sc_flags &",
         "\t    (F_A | F_B))) {",
         "\tcase 0:"]
m = SWITCH_KW.search(lines[0])
check("a two-line controlling expression is read whole",
      mask_states(scrutinee(lines, 0, m.end() - 1)) == (4, "F_A | F_B"))


print("zero because fixed, not zero because blind")

proc = SRC / "sys" / "kern" / "kern_procctl.c"
text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"),
              proc.read_text(errors="replace"), flags=re.S)
plines = text.splitlines()
found = []
for ln, line in enumerate(plines):
    m = SWITCH_KW.search(line)
    if not m:
        continue
    st = mask_states(scrutinee(plines, ln, m.end() - 1))
    if st is None:
        continue
    body, _ = switch_body(plines, ln)
    if any(DEFAULT.match(b) for b in body):
        continue
    if sum(1 for b in body if CASE.match(b)) >= st[0]:
        continue
    found.append((ln + 1, st))

check("both procctl switches still reach condition (4)",
      len(found) == 2, f"{found}")
check("they are the ASLR and PROTMAX masks",
      sorted(st[1] for _, st in found) ==
      ["P2_ASLR_ENABLE | P2_ASLR_DISABLE",
       "P2_PROTMAX_ENABLE | P2_PROTMAX_DISABLE"])
check("and (4) is what clears them -- the fix initialised the variable",
      list(sites(proc)) == [])
for ln, _ in found:
    decls = uninitialised_locals(plines, ln - 1)
    body, _ = switch_body(plines, ln - 1)
    assigned = {a.group(1) for b in body if (a := ASSIGN.match(LABEL.sub("", b)))}
    check(f"kern_procctl.c:{ln} assigns inside the switch",
          bool(assigned), "an arm that assigns nothing cannot regress")
    check(f"kern_procctl.c:{ln} has no uninitialised name to leak",
          not (decls & assigned), f"{sorted(decls & assigned)}")


print("the reporting path, which zero findings never exercise")

planted = ("void f(unsigned x)\n{\n\tint d;\n"
           "\tswitch ((x & 0x00f0) >> 4) {\n"
           "\tcase 0:\t\td = 1; break;\n"
           "\tcase 1:\t\td = 2; break;\n"
           "\t}\n\tg(d);\n}\n")
got = hits(planted)
check("the planted defect is found", len(got) == 1)
if got:
    out = format_hit("t.c", *got[0])
    check("it prints without raising", len(out) == 3)
    check("and says which mask and how wide the gap is",
          "0x00f0: 16 states, 2 case arm(s)" in out[2], out[2])
    check("and names the variable that escapes", "(d)" in out[0], out[0])


print("nfscl_fillsattr, which the read test is FOR")

nfs = SRC / "sys" / "fs" / "nfs" / "nfs_commonsubs.c"
ntext = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"),
               nfs.read_text(errors="replace"), flags=re.S)
nlines = ntext.splitlines()
reached, escaped = [], []
for ln, line in enumerate(nlines):
    m = SWITCH_KW.search(line)
    if not m:
        continue
    st = mask_states(scrutinee(nlines, ln, m.end() - 1))
    if st is None:
        continue
    body, end = switch_body(nlines, ln)
    if any(DEFAULT.match(b) for b in body):
        continue
    if sum(1 for b in body if CASE.match(b)) >= st[0]:
        continue
    decls = uninitialised_locals(nlines, ln)
    assigned = {a.group(1) for b in body
                if (a := ASSIGN.match(LABEL.sub("", b)))}
    if decls & assigned:
        reached.append((ln + 1, sorted(decls & assigned)))
        if read_after(nlines, end, decls & assigned):
            escaped.append(ln + 1)

check("two NFS switches set an uninitialised name in an arm",
      len(reached) == 2, f"{reached}")
check("neither is read after the switch, so neither reports",
      escaped == [], f"{escaped}")
check("nfs_commonsubs.c is clean", list(sites(nfs)) == [])


print()
print(f"{len(fails)} failure(s)" if fails else "all checks pass")
sys.exit(1 if fails else 0)
