#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Assert PBSD's four edits to the vendor tree are still there.

The vendor branch's base tree is hbsd/src as PBSD has it, not upstream as
it was at the snapshot -- that revision is not recoverable. For 11,337 of
the 11,341 files that differ from upstream this makes no difference, since
the difference is upstream moving on and a merge will simply take it.

For four files it does. PBSD changed them, and with base == ours a merge
takes upstream's side silently: no conflict, no message, PBSD's hundred
lines gone. There is no way to make git notice. So it is checked instead.

Run this after every upstream merge. If a marker is missing, the merge ate
it; recover the hunk from the previous commit rather than re-deriving it.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

# file -> (marker that must appear, what PBSD changed there)
MARKS = {
    "hbsd/src/Makefile.inc1":
        ("PBSD", "lib32 and Tier-2 compat architectures removed"),
    "hbsd/src/share/mk/src.opts.mk":
        ("PBSD", "no 32-bit ARM LLVM target"),
    "hbsd/src/sys/conf/kern.mk":
        ("ParanoidBSD", "freestanding kernel C++23 flags"),
    "hbsd/src/sys/conf/kmod.mk":
        ("PBSD", "C++23 module TUs, dual-link pattern"),
}


def main() -> int:
    missing = []
    for rel, (marker, what) in sorted(MARKS.items()):
        path = ROOT / rel
        if not path.is_file():
            missing.append((rel, what, "file is gone"))
            continue
        if marker.encode() not in path.read_bytes():
            missing.append((rel, what, f"no {marker!r} marker"))

    for rel, what, why in missing:
        print(f"FAIL  {rel}: {why}")
        print(f"      PBSD change here: {what}")

    if missing:
        print(f"\n{len(missing)} of {len(MARKS)} PBSD vendor edits lost.")
        print("An upstream merge takes upstream's side on these without a")
        print("conflict. Recover the hunks from the commit before the merge.")
        return 1

    print(f"PBSD vendor edits intact — all {len(MARKS)} markers present.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
