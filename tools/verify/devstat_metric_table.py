#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""devstat_arg_list[] is indexed by devstat_metric, in two files.

Fifty-three core.NullDereference findings in lib/libdevstat are one
function and one invariant.  devstat_compute_statistics() reads a
variadic list of (metric, pointer) pairs, and for each metric it decides
which type to pull off the va_list from a TABLE:

    switch (devstat_arg_list[metric].argtype) {   (devstat.c:1306)
    case DEVSTAT_ARG_UINT64:
            destu64 = (u_int64_t *)va_arg(ap, u_int64_t *);
            break;
    case DEVSTAT_ARG_LD:
            destld = (long double *)va_arg(ap, long double *);

and then, further down, which of those two pointers to write through
from a `switch (metric)' with one case per metric:

    case DSM_TOTAL_BYTES:
            *destu64 = totalbytes;                (devstat.c:1327)
    ...
    case DSM_KB_PER_TRANSFER:
            *destld = ...;

The two switches agree only while row i of the table belongs to metric
i.  The enum is in devstat.h and the table is in devstat.c, and until
this pass nothing at all connected them.  Add a metric to the middle of
the enum without adding its row here and EVERY metric after it reads
the wrong argtype: va_arg() takes a `long double *' where the caller
passed a `u_int64_t *', and a 16-byte write lands in the caller's
8-byte storage -- or the reverse, and the caller's long double is left
holding eight bytes of a totals counter.

devstat.c now carries

    _Static_assert(nitems(devstat_arg_list) == DSM_MAX, ...)

which is the LENGTH half, and is what a compiler can check.  A compiler
cannot check the ORDER: a table with the right number of rows in the
wrong order compiles.  That is what this reads, out of both files.

The analyser's fifty-three are false BECAUSE of this invariant -- it
cannot enumerate forty-five table rows to correlate the two switches,
so on its modelled path the argtype comes from one row and the case
from another.  A file that reports fifty-three findings resting on an
unwritten invariant is a file where the invariant is worth writing
down.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "hbsd" / "src"


def metrics(header: str) -> list[str]:
    """The devstat_metric enumerators, in order, DSM_MAX excluded.

    Anchored on the CLOSING `} devstat_metric', and the opening brace
    found by searching BACK from it -- devstat.h has several typedef
    enums and a forward search from the first one spans them all,
    which is how the first draft came back with DEVSTAT_MATCH_NONE as
    metric zero.
    """
    end = header.find("} devstat_metric")
    if end < 0:
        return []
    start = header.rfind("typedef enum {", 0, end)
    if start < 0:
        return []
    body = re.sub(r"/\*.*?\*/", "",
                  header[start + len("typedef enum {"):end], flags=re.S)
    out = [w.split("=")[0].strip() for w in body.split(",")]
    return [e for e in out if e and e != "DSM_MAX"]


def rows(source: str) -> list[str]:
    """The metric each devstat_arg_list row names, in table order."""
    m = re.search(r"\}\s*const devstat_arg_list\[\]\s*=\s*\{(.*?)\n\};",
                  source, re.S)
    return re.findall(r"\{\s*(DSM_\w+)\s*,", m.group(1)) if m else []


def check(header: str, source: str) -> list[str]:
    """The ways the table and the enum disagree, in words."""
    e, r = metrics(header), rows(source)
    if not e:
        return ["devstat.h: no devstat_metric enum found"]
    if not r:
        return ["devstat.c: no devstat_arg_list[] found"]
    bad = []
    if len(e) != len(r):
        bad.append(f"{len(e)} metrics but {len(r)} table rows")
        for x in e:
            if x not in r:
                bad.append(f"  {x} is in the enum and has no row")
        for x in r:
            if x not in e:
                bad.append(f"  {x} has a row and is not in the enum")
    for i, (a, b) in enumerate(zip(e, r)):
        if a != b:
            bad.append(f"row {i} is {b}, but metric {i} is {a}")
    return bad


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 if the table and the enum disagree")
    args = ap.parse_args()
    h = SRC / "lib" / "libdevstat" / "devstat.h"
    c = SRC / "lib" / "libdevstat" / "devstat.c"
    if not (h.is_file() and c.is_file()):
        print("libdevstat is not in this tree")
        return 0
    bad = check(h.read_text(errors="replace"), c.read_text(errors="replace"))
    n = len(metrics(h.read_text(errors="replace")))
    if bad:
        print(f"FAIL  devstat_arg_list[] does not match devstat_metric:")
        for b in bad:
            print(f"      {b}")
        print("      devstat_compute_statistics() would read the wrong "
              "argtype for every\n      metric at or after the first "
              "disagreement, and va_arg() would take\n      the wrong "
              "type off the caller's list.")
        return 1 if args.gate else 0
    print(f"ok    devstat_arg_list[i].metric == i for all {n} metrics")
    return 0


if __name__ == "__main__":
    sys.exit(main())
