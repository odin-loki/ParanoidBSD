#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""report.py's [triaged] marker, and the direction it must not fail in.

Missing a mark costs a re-read. Adding one to a finding nobody has read
hides it behind a claim that somebody has, which is the failure this
whole document set exists to prevent - so the tests that matter are the
ones asserting it does NOT mark.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import report  # noqa: E402

ok = True


def check(name, got, want):
    global ok
    if got == want:
        print(f"  ok   {name}")
    else:
        ok = False
        print(f"  FAIL {name}: got {got!r}, want {want!r}")


print("the marker matches on a path suffix")
# The document writes short names for paths it mentions once...
check("bare basename ref matches a full path",
      report.is_triaged("lib/libc/locale/wcsftime.c", "90"), True)
# ...and full paths where it wants to disambiguate.
check("full path ref matches",
      report.is_triaged("sys/x86/isa/clock.c", "200"), True)
# Matching is suffix-of-the-FINDING against the document's reference, not
# the other way round, so a truncated finding path does not match a longer
# reference. Findings always carry tree-relative paths, so this does not
# arise - and not marking is the safe direction when it would.
check("a truncated finding path does not match a longer ref",
      report.is_triaged("isa/clock.c", "200"), False)

print("\nand does not match what it should not")
check("right file, wrong line",
      report.is_triaged("sys/x86/isa/clock.c", "201"), False)
check("wrong file, right line",
      report.is_triaged("sys/kern/kern_proc.c", "200"), False)
check("a file the table never mentions",
      report.is_triaged("sys/kern/vfs_bio.c", "42"), False)
check("None line never matches",
      report.is_triaged("sys/x86/isa/clock.c", None), False)
# The dangerous one: `clock.c` must not match `not_clock.c`, which a
# naive endswith() would let through.
check("a longer basename does not match by endswith",
      report.is_triaged("sys/dev/xxxclock.c", "200"), False)

print("\nthe line comes out of CBMC's description, not a field")
check("desc_line reads it",
      report.desc_line({"desc": "line 200 division by zero in a / b"}), "200")
check("desc_line on something else",
      report.desc_line({"desc": "memcpy source region readable"}), None)
check("desc_line on an empty record", report.desc_line({}), None)

print("\nprose after the table is not the table")
# The failure this replaced: the scan ran from the table's heading to
# the end of the file, and 19,500 lines of writeup sit after it. The
# sentence that cited this one reads "Twelve more findings of the same
# checker are NOT COVERED HERE" - and it was being marked as read.
check("a citation in a later section is not triage",
      report.is_triaged("lib/libc/iconv/citrus_iconv.c", "100"), False)
check("...nor another from the same sentence",
      report.is_triaged("sys/netgraph/ng_parse.c", "148"), False)
# citrus_mapper.c:188 was in that same sentence until it was actually
# read; it is in the table now, and that is what moving one looks like.
check("...but one that was since read and tabled is",
      report.is_triaged("lib/libc/iconv/citrus_mapper.c", "188"), True)
# A defect that was FOUND and fixed is cited in its own section too, and
# its line number now points at something else entirely.
check("a fixed defect's writeup is not triage",
      report.is_triaged("sys/dev/bwn/if_bwn.c", "5317"), False)

print("\nthe table is actually being read")
n = len(report.triaged())
check("more than twenty entries parsed", n > 20, True)
print(f"       ({n} file:line pairs in the not-a-defect table)")

print("\n" + ("the marker is honest in both directions"
              if ok else "SOMETHING IS WRONG"))
sys.exit(0 if ok else 1)
