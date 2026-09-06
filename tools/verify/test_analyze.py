#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""The analyser must find planted bugs, or its zero means nothing.

analyze.py's first version reported "0 findings across 60 translation
units" because -analyzer-disable-all-checks turned everything off and the
-analyzer-checker flags after it did not turn anything back on. The number
was real and the check was not running.

So: four deliberate defects, one per checker family, and the run fails if
any goes unreported.
"""
from __future__ import annotations
import subprocess, sys, tempfile
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from analyze import analyze  # noqa: E402

PROBE = """#include <stdlib.h>
int leak(void){ char *p = malloc(10); return p ? 1 : 0; }
int nulldrf(int c){ int *p = 0; if (c) p = malloc(4); return *p; }
int divz(int a){ return a / 0; }
int uninit(void){ int x; return x + 1; }
"""

WANT = ["unix.Malloc", "core.NullDereference", "core.DivideZero",
        "core.uninitialized"]


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        f = Path(td) / "probe.c"
        f.write_text(PROBE)
        r = analyze({"src": str(f), "rel": "probe.c", "timeout": 120})
    got = {x["checker"] for x in r["findings"]}
    print(f"  status={r['status']}  {len(r['findings'])} finding(s)")
    for x in r["findings"]:
        print(f"    [{x['checker']}] {x['msg'][:70]}")
    fail = 0
    for w in WANT:
        hit = any(c.startswith(w) for c in got)
        print(f"  {'ok  ' if hit else 'FAIL'} {w}")
        fail += not hit
    print("\n" + ("all planted defects found" if not fail
                  else f"FAILURES: {fail} checker family/families silent"))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
