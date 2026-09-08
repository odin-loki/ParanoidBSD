#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A rule about where functions begin, checked against the analyser's own.

param_premise.py sorts every sweep finding by the function it is inside
and by who can call that function, and it finds the function by reading
the text: style(9) puts a definition's opening brace alone at column 0.
A rule like that can be a little bit wrong and still produce a table that
looks entirely reasonable, so it is checked two ways.

testdata/param_premise_shapes.c is the shapes, written out: a comment
block directly above the declarator (which hid `static' from the first
version and called two thirds of the tree's static functions exported),
a function-pointer parameter, a prototype that is not a call, a
DEVMETHOD table entry that is the address and not a call, and an
RB_GENERATE_STATIC at file scope between two functions.

The second is the analyser itself. `-analyzer-output=plist' puts an
`issue_context' on every diagnostic - the function clang says the finding
is in - so the rule can be compared against it on real files with no
sweep data needed. Over the thirty files first sampled that way it agreed
on 40 of 42 findings, and both disagreements were the macro case, where
this rule says "no function" and clang names the function the macro
generated. Saying nothing there is the intended answer: the alternative
is blaming the neighbouring function, which is what the bound on a
definition's closing brace was added to stop.
"""
from __future__ import annotations

import plistlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import param_premise as pp  # noqa: E402

ROOT = HERE.parents[1]
SRC = ROOT / "hbsd/src"
FAILURES: list[str] = []

# Three real files: one with the macro-at-file-scope shape, one whose
# findings are in a header included from it, one plain.
LIVE = ("sys/netinet/tcp_log_buf.c",
        "sys/kern/kern_condvar.c",
        "sys/geom/eli/g_eli_key_cache.c")


def check(ok: bool, what: str) -> None:
    print(("  ok    " if ok else "  FAIL  ") + what)
    if not ok:
        FAILURES.append(what)


def main() -> int:
    f = pp.File(HERE / "testdata/param_premise_shapes.c")

    print("the shapes, read as text")
    e = f.enclosing(24)                     # inside commented()
    check(e is not None and e[0] == "commented", "a definition is found")
    check(e is not None and e[2] is True,
          "and is static, though a comment block sits above the declarator")
    check(e is not None and set(e[1]) == {"sc", "flag"},
          "with both of its parameters")

    e = f.enclosing(40)                     # inside with_callback()
    check(e is not None and e[0] == "with_callback",
          "a function-pointer parameter does not truncate the declarator")
    check(e is not None and set(e[1]) == {"sc", "cmp", "n"},
          "and the pointer's own name is a parameter")
    check(e is not None and e[0] == "with_callback",
          "and a #define body's column-0 brace does not end the function")

    check(f.enclosing(44) is None,
          "a macro at file scope belongs to no function, not to the one "
          "above it")
    check(f.enclosing(12) is None, "nor does a global at file scope")

    e = f.enclosing(50)
    check(e is not None and e[0] == "exported_one" and e[2] is False,
          "an exported definition is not static")

    print()
    print("who can call it")
    check(f.uses("commented")[0] == 1,
          "commented() has one call site and its prototype is not one")
    check(f.uses("shapes_probe") == (0, 1),
          "shapes_probe() has none, and one mention: the DEVMETHOD entry")
    check(f.uses("shapes_isr") == (0, 0),
          "shapes_isr() has neither, so nothing in this file reaches it")

    print()
    print("against the analyser's own issue_context")
    if not shutil.which("clang") or not SRC.is_dir():
        print("  skip  no clang or no hbsd/src checkout here")
    else:
        sys.path.insert(0, str(HERE))
        import includes  # noqa: E402
        agree = disagree = 0
        bad = []
        for rel in LIVE:
            if not (SRC / rel).is_file():
                continue
            flags = includes.include_flags(Path(rel), arch="amd64")
            with tempfile.TemporaryDirectory() as td:
                out = Path(td) / "r.plist"
                subprocess.run(
                    ["clang", "--analyze", "-Xclang",
                     "-analyzer-output=plist", "-o", str(out)]
                    + flags + [str((SRC / rel).resolve())],
                    capture_output=True)
                if not out.is_file():
                    continue
                d = plistlib.loads(out.read_bytes())
            src = pp.File(SRC / rel)
            for diag in d.get("diagnostics", []):
                line = diag.get("location", {}).get("line")
                if line is None:
                    continue
                theirs = diag.get("issue_context")
                e = src.enclosing(line)
                mine = e[0] if e else None
                if mine == theirs or (mine is None and theirs
                                      and "_RB_" in theirs):
                    agree += 1
                else:
                    disagree += 1
                    bad.append(f"{rel}:{line} mine {mine!r} clang {theirs!r}")
        check(agree > 0, f"{agree} finding(s) compared against clang")
        check(not bad, "and the enclosing function agrees on every one")
        for b in bad[:8]:
            print(f"        {b}")

    print()
    if FAILURES:
        print(f"{len(FAILURES)} FAILED")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
