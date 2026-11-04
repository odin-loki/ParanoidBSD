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

# Deliberately several functions with several lines between them, so the
# analyser emits interleaved source context and a location regex that
# matches across newlines has something to swallow. The single-function
# version of this probe passed while 430 of 705 real findings came back
# with a location like
#
#   '   60 |  *dstlenp = len;\n      |   ~~~ ^\n/home/.../sysctl.c:110'
#
# because [^:]+ matches newlines. A probe that cannot produce the failure
# cannot report it.
PROBE = """#include <stdlib.h>
#include <string.h>

static int helper(int *p, int n)
{
        int total = 0;
        for (int i = 0; i < n; i++)
                total += p[i];
        return total;
}

int leak(void){ char *p = malloc(10); return p ? 1 : 0; }

int nulldrf(int c)
{
        int *p = 0;
        if (c)
                p = malloc(4);
        return *p + helper(p, 1);
}

int divz(int a){ return a / 0; }

int uninit(void)
{
        int x;
        int y = x + 1;
        return y;
}
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

    # Every location must be a clean file:line. This is the assertion the
    # first version did not have, and it is the one that was needed.
    import re as _re
    bad = [x for x in r["findings"]
           if not _re.fullmatch(r"[^\s:]+:\d+", x["where"])]
    print(f"  {'ok  ' if not bad else 'FAIL'} every location is a clean "
          f"file:line ({len(bad)} malformed)")
    for x in bad[:3]:
        print(f"      {x['where']!r}")
    fail += bool(bad)

    for w in WANT:
        hit = any(c.startswith(w) for c in got)
        print(f"  {'ok  ' if hit else 'FAIL'} {w}")
        fail += not hit
    # The option retry, on the file that needs it. A disjunction gives
    # the intersection of its alternatives, which is the only part that
    # is not a guess; sys/arm/arm/debug_monitor.c is `optional ddb |
    # gdb' and defines dbg_monitor_init() twice, once inside `#ifdef
    # DDB' and once outside, so with neither macro it does not compile.
    # The retry supplies one alternative and RECORDS which, so the
    # guess is in the data rather than hidden in the flags.
    import includes as _inc
    rel = "sys/arm/arm/debug_monitor.c"
    if (_inc.SRC / rel).is_file():
        r2 = analyze({"src": str(_inc.SRC / rel), "rel": rel,
                      "timeout": 200})
        ok = r2["status"] == "OK" and r2.get("opts")
        print(f"  {'ok  ' if ok else 'FAIL'} the option retry compiles "
              f"{rel.split('/')[-1]} and says with what "
              f"({r2['status']}, opts={r2.get('opts')})")
        fail += not ok
        # ...and a file that compiles without a guess must not get one.
        r3 = analyze({"src": str(_inc.SRC / "sys/xdr/xdr.c"),
                      "rel": "sys/xdr/xdr.c", "timeout": 200})
        clean = r3["status"] == "OK" and not r3.get("opts")
        print(f"  {'ok  ' if clean else 'FAIL'} ...and xdr.c compiles "
              f"with no guess at all ({r3['status']}, "
              f"opts={r3.get('opts')})")
        fail += not clean

    # The flag digest. A finding that moves between sweeps is either the
    # code changing or the command changing, and a record that carries
    # only the finding cannot tell you which - which is how
    # nfs_nfsdport.c:2683 came and went across three sweeps with nothing
    # in the file touched and no way left to check.
    rel = "sys/kern/kern_malloc.c"
    if (_inc.SRC / rel).is_file():
        job = {"src": str(_inc.SRC / rel), "rel": rel, "timeout": 200}
        d1 = analyze(dict(job)).get("flags")
        d2 = analyze(dict(job)).get("flags")
        print(f"  {'ok  ' if d1 else 'FAIL'} a record carries the flags "
              f"it was analysed with ({d1})")
        fail += not d1
        print(f"  {'ok  ' if d1 == d2 else 'FAIL'} ...and the same "
              f"question gets the same digest")
        fail += d1 != d2
        # A different architecture is a different command, and the
        # digest has to say so or it is measuring nothing.
        d3 = analyze(dict(job, arch="aarch64")).get("flags")
        print(f"  {'ok  ' if d3 and d3 != d1 else 'FAIL'} ...and a "
              f"different architecture gets a different one ({d3})")
        fail += not (d3 and d3 != d1)
        # ...and, the point of it, a SECOND PROCESS agrees. Within one
        # process incs_shim() is cached and every call sees the same
        # mkdtemp'd directory, so dropping the normalisation of those
        # paths costs nothing here and everything between two sweeps.
        code = ("import sys; sys.path.insert(0, %r)\n"
                "from analyze import analyze\n"
                "print(analyze({'src': %r, 'rel': %r, 'timeout': 200})"
                "['flags'])" % (str(Path(__file__).resolve().parent), job["src"], rel))
        try:
            d4 = subprocess.run([sys.executable, "-c", code],
                                capture_output=True, text=True,
                                timeout=600).stdout.strip()
        except (OSError, subprocess.SubprocessError):
            d4 = ""
        print(f"  {'ok  ' if d4 == d1 else 'FAIL'} ...and a second "
              f"process agrees ({d4 or 'no answer'})")
        fail += d4 != d1

    print("\n" + ("all planted defects found" if not fail
                  else f"FAILURES: {fail} checker family/families silent"))
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
