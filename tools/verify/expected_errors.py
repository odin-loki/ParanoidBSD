#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Translation units the analyser is EXPECTED not to compile, and why.

A file that does not compile contributes no findings and looks, in every
total this sweep prints, exactly like a file that is clean. That is not a
hypothetical: libexec/rtld-elf/rtld.c came back ERROR on an unresolvable
`#include "notes.h"', and the defect it was hiding cost thirty-seven boot
runs. See docs/BUILDING.md.

So the ERROR set is inventoried. Every entry here is a decision on the
record; anything erroring that is NOT here is a new hole, and anything
here that has started compiling is a stale exemption. Both are reported
by analyze.py --check-errors.

Most entries are one of three honest reasons:

  not a translation unit   the file is #included by another (arch trap.c
                           includes subr_syscall.c) and has no business
                           compiling alone.
  option-gated             it needs a kernel option no config in this
                           tree sets - KASAN, KCSAN, KMSAN, TSLOG,
                           DEVICE_POLLING, COMPAT_43TTY.
  wrong architecture       32-bit-only or arch-private helpers, against
                           an amd64 sweep.

What is NOT an honest reason is "it needs a define nobody supplied".
sys/kern/subr_param.c was in this list for exactly that - MAXUSERS,
which config(8) fakes into an option - and it is not any more. That file
sets hz, maxfiles, nbuf and maxproc, and `hz' being clamped there is an
argument this repository's triage table leans on, made about a file the
analyser had never read. One -D and it compiles.
"""
from __future__ import annotations

EXPECTED = {
    # not a translation unit: #included by another file
    "sys/kern/kern_ctf.c":          "#included by kern_linker.c",
    "sys/kern/subr_syscall.c":      "#included by each arch's trap.c",
    "sys/kern/systrace_args.c":     "generated, #included by the dtrace glue",
    "sys/kern/subr_busdma_bounce.c": "#included by each arch's busdma",
    "sys/kern/subr_devmap.c":       "arch-private, #included where used",
    "sys/kern/subr_sfbuf.c":        "arch-private sf_buf helpers",

    # option-gated: no kernel config in this tree sets these
    "sys/kern/subr_asan.c":         "needs option KASAN",
    "sys/kern/subr_csan.c":         "needs option KCSAN",
    "sys/kern/subr_msan.c":         "needs option KMSAN",
    "sys/kern/kern_tslog.c":        "needs option TSLOG",
    "sys/kern/kern_poll.c":         "needs option DEVICE_POLLING",
    "sys/kern/tty_compat.c":        "needs option COMPAT_43TTY",

    # wrong architecture for an amd64 sweep
    "sys/kern/subr_atomic64.c":     "32-bit archs only",
    "sys/kern/subr_intr.c":         "needs machine/intr.h, which amd64 has not",
    "sys/powerpc/ofw/ofw_machdep.c": "wants powerpc's <fdt.h>, absent on amd64",

    # net80211
    "sys/net80211/ieee80211_alq.c": "needs option IEEE80211_ALQ",

    # libexec/rtld-elf. The other nine translation units in this
    # directory compiled for the first time when the rtld's own include
    # flags were supplied - see includes.py - and riscv/reloc.c reported
    # a finding immediately. These six are what is left.
    "libexec/rtld-elf/aarch64/reloc.c":   "another architecture's relocations",
    "libexec/rtld-elf/arm/reloc.c":       "another architecture's relocations",
    "libexec/rtld-elf/powerpc/reloc.c":   "another architecture's relocations",
    "libexec/rtld-elf/powerpc64/reloc.c": "another architecture's relocations",
    "libexec/rtld-elf/tests/parse_integer_test.c":
        "#includes parse_integer_func.c, which the test Makefile stages",
    "libexec/rtld-elf/tests/target/target.c":
        "#includes pythagoras.h from a sibling test library",
}


def classify(errors: set[str]) -> tuple[list[str], list[str]]:
    """(unexpected, stale) for the ERROR set of one run.

    `stale' is only meaningful for files the run actually looked at, so
    the caller passes the scope's whole file set as `errors' being the
    ERROR subset of it - see analyze.py.
    """
    unexpected = sorted(e for e in errors if e not in EXPECTED)
    return unexpected, []
