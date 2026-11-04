#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Re-run clang's analyser on ONE translation unit, with the path notes.

analyze.py records WHAT the analyser found.  It does not record HOW the
analyser got there, and for this class of finding the how is the whole
question: a `core.uninitialized.ArraySubscript' whose path assumed an
extern returned zero is an artifact, and the byte-identical one whose
path took a real branch is a memory-corruption primitive.  Telling them
apart means reading the note chain.

So the sweep keeps plist-multi-file -- it carries issue_context, which
the text output does not -- and this runs the same file again with
-analyzer-output=text when a finding needs explaining.

The flags are built by calling analyze.py's own lang_flags() and
include_flags(), not by copying them.  A finding this prints is a
finding the sweep saw: if the two ever diverge, the divergence is a bug
in one of them rather than a difference in how they were invoked.

    python3 tools/verify/explain.py sys/dev/gve/gve_tx_dqo.c
    python3 tools/verify/explain.py usr.bin/login/login.c nonnull

The second argument filters to the findings whose warning line contains
it -- a checker name, a variable, a message fragment -- and prints that
report's notes and nothing else.  Without it, every finding in the file
comes back.
"""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import analyze as A  # noqa: E402


def explain(rel: str, want: str | None = None, timeout: int = 900) -> str:
    src = A.SRC / rel
    if not src.is_file():
        raise SystemExit(f"no such translation unit: {rel}")
    arch = A.arch_of(rel)
    # The shim is built in this process rather than inherited: explain.py
    # is run by hand, one file at a time, and a stale PBSD_INCS from a
    # previous sweep would silently change the include path.
    A.incs_shim(arch)
    flags = [*A.lang_flags(src, rel), *A.include_flags(src, arch)]
    cmd = ["clang", "--analyze", "-Xclang", "-analyzer-output=text",
           *flags, str(src), "-o", "/dev/null"]
    p = subprocess.run(cmd, capture_output=True, text=True,
                       timeout=timeout, cwd="/tmp")
    out = p.stderr
    if want is None:
        return out
    # A report is its warning line plus every note until the next
    # warning.  Selecting on the warning line alone would drop the
    # notes, which are the reason to run this at all.
    keep: list[str] = []
    emit = False
    for line in out.splitlines():
        if ": warning: " in line or ": error: " in line:
            emit = want in line
        if emit:
            keep.append(line)
    return "\n".join(keep)


def main() -> int:
    if not 2 <= len(sys.argv) <= 3:
        print(__doc__, file=sys.stderr)
        return 2
    os.chdir(A.ROOT if hasattr(A, "ROOT") else Path(__file__).resolve().parents[2])
    text = explain(sys.argv[1], sys.argv[2] if len(sys.argv) == 3 else None)
    print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
