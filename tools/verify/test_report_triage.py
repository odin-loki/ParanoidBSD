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

print("\nthe continuation form attaches to the path on its left")
# The document writes `dis_tables.c:6419`, `:6447` when one row names
# several lines in one file. 64 of those across 31 rows matched nothing
# at all until the path group was made optional.
check("a bare `:NNN` continues the row's last path",
      report.is_triaged("sys/cddl/dev/dtrace/x86/dis_tables.c", "6447"), True)
# ...and "last path" means the nearest one to its LEFT, not the row's
# first: this row opens on nfs_clrpcops.c, names nfs_clstate.c:4960
# mid-sentence, and then continues with `:5070`/`:5071`.
check("it is the nearest path to the left, not the row's first",
      report.is_triaged("sys/fs/nfsclient/nfs_clstate.c", "5071"), True)
check("...and not the row's opening file",
      report.is_triaged("sys/fs/nfsclient/nfs_clrpcops.c", "5071"), False)

print("\nextern-driven is decided on CBMC's evidence, not on a name list")


def rec(fn, desc, no_body=None, linkage="exported", f="lib/libc/x.c"):
    r = {"file": f, "function": fn, "linkage": linkage,
         "failures": [{"name": "p", "desc": desc}]}
    if no_body is not None:
        r["no_body"] = no_body
    return r


# The case the name list could not reach: __gedf2 is not in EXTERN_DRIVEN
# and never would be, because the list has to be extended by hand for
# every new callee CBMC cannot model. CBMC says `no body for function
# __softfloat_float64_le' and then names that same return in the
# expression that overflows, which is the whole argument.
check("a no_body callee named in the expression",
      report.bucket(rec("__gedf2",
                        "line 18 arithmetic overflow on signed - in "
                        "return_value___softfloat_float64_le - 1",
                        ["__softfloat_float64_le"])),
      "extern-driven (an unmodelled return, unconstrained)")
# The direction that matters. An unmodelled callee SOMEWHERE in the
# function is not evidence about THIS expression - almost every libc
# function has one - so a failure on a parameter stays where a person
# will read it. s_cosf's `-n' is exactly this: __kernel_rem_pio2 has no
# body, but `n' is not spelled as its return and the rule declines to
# guess.
check("a no_body callee NOT named in the expression",
      report.bucket(rec("cosf",
                        "line 73 arithmetic overflow on signed unary "
                        "minus in -n",
                        ["__kernel_rem_pio2"])),
      "EXPORTED, arithmetic - READ THESE")
# A record from before the driver recorded no_body carries no evidence
# either way, so it is judged the old way rather than judged wrongly.
check("no no_body field falls back to the name list (in)",
      report.bucket(rec("clock", "line 9 arithmetic overflow on signed - "
                                 "in return_value_getrusage - 1")),
      "extern-driven (an unmodelled return, unconstrained)")
check("no no_body field falls back to the name list (out)",
      report.bucket(rec("__gedf2", "line 18 arithmetic overflow on signed "
                                   "- in return_value_x - 1")),
      "EXPORTED, arithmetic - READ THESE")
# Why the spelling alone is not the evidence, and the no_body list is.
# CBMC writes `return_value_<f>' for EVERY call whose return is used,
# whether or not it had a body for <f> - remove_returns rewrites the
# call, it does not report a missing model. Run 33 measures it: of the
# 180 `return_value_X' names in FAILED records that also carry a
# no_body list, 94 name an X the list does not, and they are inlines
# with bodies right here in the tree - __curthread, get_pcpu,
# _tcb_get, _citrus_region_offset, __log2. The `$0'/`$1' suffix CBMC
# appends to tell two call sites in one frame apart is the giveaway:
# a stub for an unmodelled callee has one nondeterministic return, not
# a numbered pair.
#
# So a rule that read extern-driven off the spelling would have hidden
# those 94 - DELAY() below among them, where the overflow is
# sched_pin()'s `td_pinned + 1' on a counter CBMC cannot bound but a
# person can. That is a finding to read, not an absent model.
check("return_value_ of an INLINE is not an absent model",
      report.bucket(rec("DELAY", "line 179 arithmetic overflow on signed + "
                                 "in return_value___curthread->td_pinned + 1",
                        ["panic"], f="sys/x86/x86/delay.c")),
      "EXPORTED, arithmetic - READ THESE")
# ... and the call-site suffix does not defeat the match when the
# callee IS unmodelled: `return_value_sprintf$0' contains the name the
# list contributes.
check("a no_body callee matches through CBMC's call-site suffix",
      report.bucket(rec("identify_arm_cpu",
                        "line 40 arithmetic overflow on signed + in "
                        "return_value_sprintf$0 + 1",
                        ["sprintf"], f="sys/arm/arm/identcpu-v6.c")),
      "extern-driven (an unmodelled return, unconstrained)")
# static still wins: its callers are all in the file, which is a stronger
# statement than anything about a callee.
check("static is decided before extern-driven",
      report.bucket(rec("__gedf2",
                        "line 18 arithmetic overflow on signed - in "
                        "return_value___softfloat_float64_le - 1",
                        ["__softfloat_float64_le"], linkage="static")),
      "static (callers constrain the domain - deferred)")

print("\nthe static bucket's claim is checked, not assumed")
# The `static' bucket defers on one claim: the callers are all in this
# file, so they narrow the domain. For a function whose ADDRESS is taken
# that claim is false -- the callers are whoever holds the pointer.
# _UTF8_mbrtowc is `l->__mbrtowc' in lib/libc/locale/utf8.c, reached
# from mbrtowc(3) with an application's bytes, which is the opposite of
# constrained. Run 33: 55 of the 254 records in this bucket are one of
# these, 22%.
check("an address-taken static is not deferred",
      report.bucket(rec("_UTF8_mbrtowc",
                        "line 179 arithmetic overflow on signed shl in "
                        "wch << 6",
                        linkage="static", f="lib/libc/locale/utf8.c")),
      "STATIC but its address is taken - READ THESE")
# ...and one whose name never appears in a value position still is.
check("a static with only call sites stays deferred",
      report.bucket(rec("acpi_pci_link_search_irq",
                        "line 9 arithmetic overflow on signed + in x + 1",
                        linkage="static",
                        f="sys/dev/acpica/acpi_pci_link.c")),
      "static (callers constrain the domain - deferred)")
# A file the report cannot read is absence of evidence, not evidence:
# the old, conservative bucket.
check("an unreadable file falls back to deferred",
      report.bucket(rec("nosuchfn",
                        "line 9 arithmetic overflow on signed + in x + 1",
                        linkage="static", f="lib/libc/no/such/file.c")),
      "static (callers constrain the domain - deferred)")

print("\nan index out of range is not a missing precondition")
# bucket()'s own comment said an array bound does not belong in a bucket
# labelled "a missing precondition"; it was there because PTR_WORDS has
# to contain "array " and "object" for CBMC's pointer checks. Splitting
# run 33's 1,781: 986 have a pointer-shaped bound AND a null, 542 a
# pointer-shaped bound only, 206 region/leak, 11 null only -- and 36 an
# INDEX. Reading those found talkd's three `> NTYPES'.
check("an array subscript out of range is read, not deferred",
      report.bucket(rec("print_request",
                        "line 61 array 'types' upper bound in "
                        "types[(signed long int)mp->type]",
                        f="libexec/talkd/print.c")),
      "an INDEX out of its array's range - READ THESE")
# An unconstrained POINTER is modelled as pointing at a zero-size
# object, so any arithmetic on it is "outside object bounds" -- the
# same missing precondition as NULL, differently spelled.
check("a pointer-shaped bound is still a precondition",
      report.bucket(rec("strcat",
                        "line 18 pointer arithmetic: pointer outside "
                        "object bounds in s + (signed long int)len",
                        f="lib/libc/aarch64/string/strcat.c")),
      "pointer/memory (a missing precondition, not a bug)")
# ...and a record carrying both: the null explains the subscript too.
both = {"file": "x/y.c", "function": "f", "linkage": "exported",
        "failures": [
            {"name": "a",
             "desc": "line 3 dereference failure: pointer NULL in p->q"},
            {"name": "b",
             "desc": "line 3 array 'tbl' upper bound in "
                     "tbl[(signed long int)i]"}]}
check("a null beside the subscript decides it",
      report.bucket(both),
      "pointer/memory (a missing precondition, not a bug)")
# CBMC's own model of pipe(2) is not the tree's array.
check("__CPROVER_pipes is the library's array, not the tree's",
      report.bucket(rec("__sread",
                        "line 74 array '__CPROVER_pipes'[].data upper "
                        "bound in __CPROVER_pipes[(signed long int)"
                        "fildes].data[(signed long int)i]",
                        f="lib/libc/stdio/stdio.c")),
      "pointer/memory (a missing precondition, not a bug)")

print("\nthe table is actually being read")
n = len(report.triaged())
check("more than twenty entries parsed", n > 20, True)
print(f"       ({n} file:line pairs in the not-a-defect table)")

print("\n" + ("the marker is honest in both directions"
              if ok else "SOMETHING IS WRONG"))
sys.exit(0 if ok else 1)
