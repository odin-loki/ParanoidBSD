#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Each rejection reason in check_port_candidates.py, on a synthetic tree.

The tool's value is entirely in the reasons it gives, so each one gets a
file built to trigger it and nothing else. Run against the real tree it
agrees with what this session learned the hard way - the five files that
stopped run 28 come back "included by name elsewhere", and the twelve
lib/libgcc_s names come back "named by lib/libgcc_s/Makefile" - but a real
tree cannot show that a reason fires for the RIGHT reason and only then.

Run: python3 tools/test_check_port_candidates.py
"""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

TOOL = Path(__file__).resolve().parent / "check_port_candidates.py"


def build_tree(root: Path) -> None:
    src = root / "hbsd" / "src"
    lib = src / "lib" / "demo"
    (lib / "sub").mkdir(parents=True)
    (src / "lib" / "other").mkdir(parents=True)

    # passes everything
    (lib / "clean.c").write_text("int clean(void) { return 0; }\n")
    # included by name
    (lib / "template.c").write_text("/* inlined by includer.c */\n")
    (lib / "includer.c").write_text('#include "template.c"\n')
    # not named by any Makefile in the scope
    (lib / "orphan.c").write_text("int orphan(void) { return 0; }\n")
    # conditional
    (lib / "maybe.c").write_text("int maybe(void) { return 0; }\n")
    # shadowed: same basename in a sibling directory
    (lib / "dup.c").write_text("int dup1(void) { return 0; }\n")
    (lib / "sub" / "dup.c").write_text("int dup2(void) { return 0; }\n")
    # named by a Makefile outside the scope
    (lib / "shared.c").write_text("int shared(void) { return 0; }\n")

    (lib / "Makefile").write_text(
        "SRCS+= clean.c includer.c dup.c shared.c \\\n"
        "\ttemplate.c\n"
        ".if ${MK_SOMETHING} != \"no\"\n"
        "SRCS+= maybe.c\n"
        ".endif\n")
    (lib / "sub" / "Makefile.inc").write_text("SRCS+= dup.c\n")
    (src / "lib" / "other" / "Makefile").write_text(
        ".PATH: ${SRCTOP}/lib/demo\nSRCS+= shared.c\n")


def run(root: Path, *extra: str) -> str:
    # The tool locates the tree as parents[1]/hbsd/src of its own path, so
    # it is copied into the synthetic root rather than pointed at it.
    tools = root / "tools"
    tools.mkdir(exist_ok=True)
    (tools / TOOL.name).write_bytes(TOOL.read_bytes())
    p = subprocess.run(
        [sys.executable, str(tools / TOOL.name), "--scope", "lib/demo",
         "--show-rejected", *extra],
        capture_output=True, text=True)
    if p.returncode != 0:
        raise SystemExit(f"tool failed: {p.stderr}")
    return p.stdout


def reasons(out: str) -> dict[str, list[str]]:
    got: dict[str, list[str]] = {}
    cur = None
    for line in out.splitlines():
        if line.startswith("  lib/demo/"):
            cur = line.strip()
            got.setdefault(cur, [])
        elif line.startswith("      ") and cur:
            got[cur].append(line.strip())
    return got


def main() -> int:
    failures = []

    def check(label, cond, detail=""):
        print(f"  {'ok  ' if cond else 'FAIL'} {label}")
        if not cond:
            failures.append(f"{label}: {detail}")

    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        build_tree(root)
        out = run(root)
        got = reasons(out)
        acc = [k for k, v in got.items() if not v]

        print("tree-side reasons")
        check("clean.c is accepted", "lib/demo/clean.c" in acc, out)
        check("template.c: included by name",
              any("included by name" in r
                  for r in got.get("lib/demo/template.c", [])), out)
        check("orphan.c: not named by any Makefile in scope",
              any("no Makefile in the scope names it" in r
                  for r in got.get("lib/demo/orphan.c", [])), out)
        check("maybe.c: inside an .if",
              any(".if" in r for r in got.get("lib/demo/maybe.c", [])), out)
        check("dup.c: shadowed by the sibling copy",
              any("same basename" in r
                  for r in got.get("lib/demo/dup.c", [])), out)
        check("shared.c: named outside the scope",
              any("lib/other/Makefile" in r
                  for r in got.get("lib/demo/shared.c", [])), out)
        check("includer.c is still acceptable itself",
              "lib/demo/includer.c" in acc, out)

        print("\nwith a pass report")
        rep = root / "rep.json"
        rep.write_text(json.dumps({"records": [
            {"source": "lib/demo/clean.c", "edits": 0,
             "ir": {"equal": True, "abi_equal": True}},
            {"source": "lib/demo/includer.c", "edits": 3,
             "ir": {"equal": True, "abi_equal": True}},
        ]}))
        out2 = run(root, "--report", str(rep))
        acc2 = [k for k, v in reasons(out2).items() if not v]
        check("clean.c survives the report", "lib/demo/clean.c" in acc2, out2)
        check("includer.c dropped for having edits",
              "lib/demo/includer.c" not in acc2, out2)
        check("the note about the oracle is gone when a report is given",
              "no --report" not in out2, out2)

    print()
    if failures:
        print(f"{len(failures)} FAILED")
        for f in failures:
            print(f"  {f}")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
