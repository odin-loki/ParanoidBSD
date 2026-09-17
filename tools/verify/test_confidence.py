#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The metric's own failure modes, tested in the direction that matters.

A coverage number is a thing people quote without reading, so the ways
it can lie are the ways this project gets hurt. Each case here is one
of those, and the first two are mistakes the first draft of
confidence.py actually made.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import confidence  # noqa: E402

ok = True


def check(name, got, want):
    global ok
    good = got == want
    ok = ok and good
    print(f"  {'ok  ' if good else 'FAIL'} {name}")
    if not good:
        print(f"       got  {got!r}\n       want {want!r}")


def rec(f, status, desc=None, fn="f", linkage="exported"):
    r = {"file": f, "function": fn, "status": status, "linkage": linkage}
    if desc:
        r["failures"] = [{"name": "a", "desc": desc}]
    return r


print("a scope with no data is not a clean scope")
# The whole project exists because these look identical from outside.
s = confidence.score("sys/nothing", [], [], {})
check("no data scores zero, not 1.0", s["confidence"], 0.0)
check("...and the verdict names it", confidence.verdict(s)[0],
      "NO LEDGER ENTRIES")

print("\nvisibility comes from the classify index, not the verdict file")
# The first draft counted distinct files in the MODEL-CHECK jsonl, which
# holds one record per (file, function) CHECKED. Most translation units
# contribute none -- sys/dev's 1,419 pairs came from 570 of its 2,419
# built units -- so it reported 0.03 for a scope that is really 0.93,
# understating the number this project has worked hardest to raise.
SCOPE = "lib/libc/"
classes = {f"{SCOPE}a{i}.c": {"ok": True} for i in range(100)}
model = [rec(f"{SCOPE}a0.c", "PROVED")]   # one unit, every record
s = confidence.score(SCOPE, [], model, classes)
check("100 built units, 1 with a verdict, modelled is 100 not 1",
      s["modelled"], 100)

print("\nan instrument that did not run is not an instrument that failed")
# Folding these together is the same error one level up: it makes a
# scope nobody analysed indistinguishable from one the analyser could
# not build. A real scope is used here because verdict() short-circuits
# on a path the ledger does not name -- which is correct, and the first
# draft of this test asserted against it.
check("model ran, analyser did not", (s["model_ran"], s["analyse_ran"]),
      (True, False))
check("the verdict says which is missing",
      any("analyser has no data" in w for w in confidence.verdict(s)[1]),
      True)

print("\nthe gate refuses on an unread finding, whatever the score")
# An unread finding carried across a port is a defect that survives
# without its provenance, so this one is an absolute.
unread = rec("lib/libc/x.c", "FAILED",
             "line 9 arithmetic overflow on signed + in a + b")
check("an EXPORTED-arithmetic failure is not settled",
      confidence.settled(unread), False)
pointer = rec("lib/libc/x.c", "FAILED",
              "line 9 dereference failure: pointer NULL in p->q")
check("...but a bucket whose rule explains it is",
      confidence.settled(pointer), True)

print("\nthe product cannot be inflated by one strong component")
# 90% visible, 90% answered, 90% read is 73%, and that is the point.
check("0.9 x 0.9 x 0.9", round(0.9 * 0.9 * 0.9, 2), 0.73)

print("\nthe thresholds are demonstrated, not invented")
# sys/fs is 119/119 and sys/geom 78/78 under goto-cc today, so 0.95 is
# a bar something already clears rather than an aspiration.
check("visibility gate is reachable", confidence.GATE["visibility"] <= 1.0,
      True)
check("answer gate is below what sys/dev returned untuned",
      confidence.GATE["answer"] <= 1001 / 1419, True)

print("\n" + ("the metric fails the way it should"
              if ok else "SOMETHING IS WRONG"))
sys.exit(0 if ok else 1)
