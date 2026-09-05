#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Check that scripts are executable and that data files are not.

The vendored tree arrived as a flat snapshot with every file mode 100644 -
no executable bit anywhere in 107,357 files. FreeBSD's build runs its own
scripts directly, so buildworld stops at the first one: contrib/bmake's
boot-strap could not exec ./configure.

Two rules, both mechanically checkable:

  * a text file starting with "#!" is a script and must be executable;
  * a binary file must not be, whatever its first two bytes happen to be.

That second rule is not hypothetical. Marking every "#!" file executable
caught two test *fixtures* - a bc error case and a file(1) test input - whose
leading bytes are "#!" by coincidence.
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def tracked() -> list[tuple[str, str]]:
    out = subprocess.run(
        ["git", "ls-files", "-s", "hbsd/src", "tools", "kde"],
        cwd=ROOT, capture_output=True, text=True, check=True,
    ).stdout
    rows = []
    for line in out.splitlines():
        if not line.strip():
            continue
        meta, path = line.split("\t", 1)
        rows.append((meta.split()[0], path))
    return rows


def is_binary(blob: bytes) -> bool:
    return b"\0" in blob[:8000]


def main() -> int:
    missing_x: list[str] = []
    stray_x: list[str] = []
    for mode, path in tracked():
        f = ROOT / path
        try:
            head = f.open("rb").read(8000)
        except OSError:
            continue
        shebang = head.startswith(b"#!")
        binary = is_binary(head)
        if shebang and not binary and mode != "100755":
            missing_x.append(path)
        if binary and mode == "100755":
            stray_x.append(path)

    for p in missing_x[:20]:
        print(f"FAIL  {p} starts with #! but is not executable")
    if len(missing_x) > 20:
        print(f"      ... and {len(missing_x) - 20} more")
    for p in stray_x[:20]:
        print(f"FAIL  {p} is binary but marked executable")

    if missing_x or stray_x:
        print(f"\n{len(missing_x)} script(s) not executable, "
              f"{len(stray_x)} binary file(s) executable.")
        print("Fix with: git update-index --chmod=+x -- <path>")
        return 1
    print("exec bits OK — every tracked script is executable, "
          "no binary file is.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
