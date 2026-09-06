#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Linkage and signature class must be read off the goto model correctly.

Two things this file guards, and both were wrong once:

  LINKAGE.  goto-instrument prints, in this order,

      Symbol......: xtob
      Type........: char (signed int c)
      Value.......: irep(...)
      Flags.......: lvalue file_local

  Flags comes AFTER Type. The first regex captured only what lay BETWEEN
  Symbol and Type and looked for file_local there, so it never saw a flag
  at all: 26,548 of 26,607 kernel functions came back "exported", and the
  59 that did not were accidents. Two static inline helpers - md5c.c's
  rol32 and subr_blist.c's bitrange - were then presented as exported API
  with arithmetic defects in their whole signature domain. The linkage is
  not a label; it decides whether a modular finding is a defect or a
  missing precondition, so getting it wrong mislabels the findings.

  CLASS.  A pointer parameter makes an unguarded modular check unsound,
  and so does an extern file-scope pointer the body touches - getchar()
  takes no arguments and still dereferences __stdinp.

The probe is compiled by the real code path (goto-cc, then
goto-instrument --show-symbol-table, then model_one's parsing), not by a
fixture of captured output, because it is the parsing of real output that
was broken.
"""
from __future__ import annotations
import sys, tempfile
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from classify import model_one  # noqa: E402

PROBE = """
extern char *envp;

static int helper(int a, int b) { return a + b; }
int exported(int a) { return helper(a, 1); }

static char *sbuf(char *p) { return p + 1; }
char *also_exported(char *p) { return sbuf(p); }

void novoid(void) { }
int reads_extern(void) { return envp == 0 ? 0 : 1; }
double scalarish(double x, long y) { return x + (double)y; }
"""

WANT_LINKAGE = {
    "helper": "static",
    "sbuf": "static",
    "exported": "exported",
    "also_exported": "exported",
    "novoid": "exported",
    "reads_extern": "exported",
    "scalarish": "exported",
}

WANT_CLASS = {
    "helper": "SCALAR",
    "exported": "SCALAR",
    "sbuf": "POINTER",
    "also_exported": "POINTER",
    "novoid": "VOID",
    # takes nothing, touches an extern pointer: not soundly checkable
    # unguarded, which is the whole reason GLOBALPTR exists.
    "reads_extern": "GLOBALPTR",
    "scalarish": "SCALAR",
}


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        src = Path(td) / "probe.c"
        src.write_text(PROBE)
        r = model_one({"src": str(src), "rel": "probe.c",
                       "outdir": td, "timeout": 180})

    if not r.get("ok"):
        print("FAIL could not build the probe's goto model:")
        print("    " + str(r.get("error"))[:400])
        return 1

    fns = r["functions"]
    linkage = r["linkage"]
    fail = 0

    print(f"  probe modelled: {len(fns)} function(s)")

    for name, want in WANT_LINKAGE.items():
        got = linkage.get(name, "<absent>")
        ok = got == want
        print(f"  {'ok  ' if ok else 'FAIL'} linkage {name}: {got}"
              + ("" if ok else f" (want {want})"))
        fail += not ok

    for name, want in WANT_CLASS.items():
        got = fns.get(name, "<absent>")
        ok = got == want
        print(f"  {'ok  ' if ok else 'FAIL'} class   {name}: {got}"
              + ("" if ok else f" (want {want})"))
        fail += not ok

    # The bug was not "some symbol was mislabelled", it was "nothing was
    # ever labelled static". A run where every function is exported is the
    # exact shape of the failure and is worth naming as such.
    # Counted over the probe's own functions only: goto-cc links in model
    # functions of its own, and this assertion should answer for the probe
    # rather than for whatever CBMC's library happens to contain.
    statics = [k for k in WANT_LINKAGE if linkage.get(k) == "static"]
    ok = len(statics) == 2
    print(f"  {'ok  ' if ok else 'FAIL'} {len(statics)} of "
          f"{len(WANT_LINKAGE)} probe function(s) seen as static (want 2)")
    fail += not ok

    print("\n" + ("linkage and classes read correctly" if not fail
                  else f"FAILURES: {fail}"))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
