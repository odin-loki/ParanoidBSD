#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every directory under sys/ is in some analyse shard of pbsd-verify.yml.

The sweep runs as one script on a workstation and as a sharded pipeline on
a 4-core runner, and the shards are written out by hand because analyze.py
takes scopes and not exclusions. Hand-written means a directory can be left
out, and a directory left out is a scope that reports ZERO FINDINGS while
looking exactly like one that is clean.

The first version of the shard list missed fourteen, including
sys/hardenedbsd - which is PBSD's own code, and the last place a silent
gap belongs. So this is a gate rather than a comment.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WF = ROOT / ".github/workflows/pbsd-verify.yml"
SYS = ROOT / "hbsd/src/sys"


def main() -> int:
    if not WF.is_file() or not SYS.is_dir():
        print(f"FAIL  cannot find {WF} or {SYS}")
        return 1
    text = WF.read_text()
    # Only the analyse matrix; the model and classify jobs use whole-tree
    # scopes and are not sharded by directory.
    block = text.split("  analyze:", 1)[-1].split("\n  lints:", 1)[0]
    # Not \S+: the last scope on a quoted YAML line is `sys/cddl' }`
    # and \S+ takes the quote with it, so three directories that were
    # covered read as missing on this script's first run.
    covered = set(re.findall(r"--scope\s+([^\s'\"}]+)", block))
    top = {f"sys/{p.name}" for p in SYS.iterdir() if p.is_dir()}
    missing = sorted(
        d for d in top
        if d not in covered and "sys" not in covered
        and not any(d.startswith(c + "/") for c in covered))
    if missing:
        print(f"FAIL  {len(missing)} directory(ies) under sys/ are in no "
              "analyse shard:")
        for d in missing:
            print(f"      {d}")
        print("\n      A scope nobody analyses reports zero findings and")
        print("      looks exactly like one that is clean. Add it to the")
        print("      `rest` shard in .github/workflows/pbsd-verify.yml.")
        return 1
    print(f"every one of the {len(top)} directories under sys/ is in a shard")
    return 0


if __name__ == "__main__":
    sys.exit(main())
