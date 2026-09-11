#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Cases for the devstat_arg_list ordering check.

Every case is a synthetic pair of files built to trigger exactly one
verdict, plus the real tree at the end -- a checker that says `ok' to
everything is a checker that says nothing.
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import devstat_metric_table as d  # noqa: E402

FAIL = 0


def check(cond: bool, what: str) -> None:
    global FAIL
    print(f"  {'ok  ' if cond else 'FAIL'}  {what}")
    if not cond:
        FAIL += 1


def hdr(*names: str) -> str:
    """A devstat.h with a decoy enum in front of the real one."""
    return ("typedef enum {\n"
            "\tDEVSTAT_MATCH_NONE = 0x00,\n"
            "\tDEVSTAT_MATCH_TYPE = 0x01\n"
            "} devstat_match_flags;\n\n"
            "typedef enum {\n"
            + "".join(f"\t{n},\n" for n in names)
            + "\tDSM_MAX\n} devstat_metric;\n")


def src(*names: str) -> str:
    return ("struct devstat_args {\n\tdevstat_metric metric;\n"
            "\tdevstat_arg_type argtype;\n} const devstat_arg_list[] = {\n"
            + "".join(f"\t{{ {n}, DEVSTAT_ARG_UINT64 }},\n" for n in names)
            + "};\n")


THREE = ("DSM_NONE", "DSM_TOTAL_BYTES", "DSM_TOTAL_TRANSFERS")

print("== the enum is read past the decoy in front of it")
check(d.metrics(hdr(*THREE)) == list(THREE),
      "the real enum, not the devstat_match_flags one above it")
check("DSM_MAX" not in d.metrics(hdr(*THREE)),
      "...and DSM_MAX is the count, not a metric")
check(d.rows(src(*THREE)) == list(THREE), "the table rows, in order")

print("\n== agreement")
check(d.check(hdr(*THREE), src(*THREE)) == [], "in order and complete")

print("\n== a metric added to the enum and not to the table")
_bad = d.check(hdr("DSM_NONE", "DSM_NEW", "DSM_TOTAL_BYTES",
                   "DSM_TOTAL_TRANSFERS"), src(*THREE))
check(any("4 metrics but 3 table rows" in b for b in _bad),
      "the count is reported")
check(any("DSM_NEW is in the enum and has no row" in b for b in _bad),
      "...and the metric is named")
check(any("row 1 is DSM_TOTAL_BYTES, but metric 1 is DSM_NEW" in b
          for b in _bad),
      "...and so is the first row that shifted -- which is the one that "
      "makes va_arg take the wrong type")

print("\n== a row added to the table and not to the enum")
_bad = d.check(hdr(*THREE), src("DSM_NONE", "DSM_NEW", "DSM_TOTAL_BYTES",
                                "DSM_TOTAL_TRANSFERS"))
check(any("DSM_NEW has a row and is not in the enum" in b for b in _bad),
      "the orphan row is named")

print("\n== the same rows in the wrong order, which compiles")
_bad = d.check(hdr(*THREE), src("DSM_NONE", "DSM_TOTAL_TRANSFERS",
                                "DSM_TOTAL_BYTES"))
check(len(_bad) == 2 and all(b.startswith("row ") for b in _bad),
      "two swapped rows, and no length complaint -- this is the half "
      "the _Static_assert in devstat.c cannot see")

print("\n== and a file that is not there says so rather than passing")
check(d.check("", src(*THREE)) == ["devstat.h: no devstat_metric enum found"],
      "a header with no enum")
check(d.check(hdr(*THREE), "") == ["devstat.c: no devstat_arg_list[] found"],
      "a source with no table")

print("\n== against the real tree")
_r = subprocess.run([sys.executable,
                     str(Path(__file__).resolve().parent /
                         "devstat_metric_table.py"), "--gate"],
                    capture_output=True, text=True)
check(_r.returncode == 0, "the gate passes on the tree as it stands")
check("45 metrics" in _r.stdout,
      f"...over all forty-five of them ({_r.stdout.strip()})")

print()
if FAIL:
    print(f"{FAIL} check(s) failed")
    sys.exit(1)
print("all checks passed")
