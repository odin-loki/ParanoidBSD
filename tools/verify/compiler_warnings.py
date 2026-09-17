#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every translation unit compiled with a warning set the build does not use.

The kernel builds with -Wall and a long list of -Wno-.  That is the right
call for a build - a warning nobody can fix is a warning everybody learns
to scroll past - and it is the wrong call for an instrument, because the
suppressions are where the defects are.

This is the cheapest thing in the tier and it covers a class nothing else
here can see.  -Wconversion is the whole of it: an implicit narrowing is
not a path property, so CBMC's per-function proof never states it and
clang's path engine never reports it, and it is the mechanism behind
`ent_size` (three bits) indexing `dw[4]`, behind `es_vlangroup` (signed
int) indexing `vid[16]`, and behind every `int` parameter that arrives
from an ioctl and is stored into a `uint8_t` register field.

Two tiers, because the volume forces it.  SIGNAL is the warnings that are
nearly always a defect - a null dereference the compiler PROVED, a cast
that increases alignment, a format string that does not match its
arguments.  ADVISORY is -Wconversion and its family: a real class, at a
volume that has to be read a subsystem at a time rather than gated on.
Both are recorded.  Only SIGNAL gates.  A driver that gates on 5,000
findings gates on nothing, because the first thing anybody does is turn
it off.
"""
from __future__ import annotations

import argparse
import functools
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbsd_breadth as B  # noqa: E402

TOOL = "compiler-warnings"
VERSION = 1
INSTALL = "apt-get install -y clang gcc   # both are usually already here"

# The set the task asks for, plus what -Wall and -Wextra drag in.
WARNSET = ["-Wall", "-Wextra", "-Wconversion", "-Wsign-conversion",
           "-Wshadow", "-Wstrict-aliasing=2", "-Wformat=2", "-Wcast-align",
           "-Wnull-dereference", "-Wduplicated-cond", "-Wlogical-op"]

# ...minus the ones that are a house style and not a defect.  Each of
# these is a warning the FreeBSD build already turns off, for a reason
# that is still the reason.
OFF = [
    # style(9) declares K&R-era prototypes and unused method arguments on
    # purpose; every device method takes the arguments its table declares.
    "-Wno-unused-parameter",
    # `struct foo f = { 0 };' is the kernel's zero-initialiser.
    "-Wno-missing-field-initializers",
    # Every printf(9) wrapper takes its format from a caller.
    "-Wno-format-nonliteral",
    # -Wformat=2 wants a format string on every printf; panic("oops") is
    # not a defect.
    "-Wno-format-zero-length",
    # sys/sys/cdefs.h's __unused and the kernel's designated initialisers.
    "-Wno-unused-function",
    # -Wextra on C: `enum e x; switch (x)' over a table-driven enum.
    "-Wno-sign-compare",
]

# gcc-only.  Not silently dropped: what a run did NOT check is the thing
# this whole document set exists to keep visible, so the meta record
# carries the list and the summary prints it.
GCC_ONLY = {"-Wduplicated-cond", "-Wlogical-op"}

# A warning here is a defect or a portability bug, not a matter of taste.
SIGNAL = {
    "-Wnull-dereference",          # the compiler PROVED the deref
    "-Wstrict-aliasing",           # UB, and it miscompiles under -O2
    # -Wformat and -Wformat-extra-args are NOT here, and this is the one
    # entry that is about the COMPILER rather than the code.  printf(9)
    # has %b (a value and a decoding string), %D, %r and %y, and upstream
    # clang does not know them - FreeBSD's clang has -fformat-extensions
    # and this one does not, so `clang -fsyntax-only -fformat-extensions'
    # is "unknown argument".  All 32 format findings on the measured sweep
    # were %b in sys/x86/x86/identcpu.c and sys/geom/raid/md_intel.c, and
    # all 32 were wrong.  They are still RECORDED, as advisory; they do
    # not gate.  On a FreeBSD toolchain, put them back.
    "-Wformat-insufficient-args",
    "-Wformat-insufficient-args",
    "-Wformat-security",
    "-Wformat-truncation",         # two constants that do not fit
    "-Wformat-overflow",
    "-Wuninitialized",
    "-Wsometimes-uninitialized",
    "-Wconditional-uninitialized",
    "-Wmaybe-uninitialized",
    "-Wreturn-type",
    "-Wimplicit-function-declaration",
    "-Wint-conversion",
    "-Wincompatible-pointer-types",
    "-Warray-bounds",
    "-Wshift-count-overflow",
    "-Wshift-count-negative",
    "-Wshift-negative-value",
    "-Wshift-overflow",
    "-Wtautological-compare",
    "-Wtautological-constant-out-of-range-compare",
    "-Wtautological-pointer-compare",
    "-Wtautological-unsigned-zero-compare",
    "-Wduplicated-cond",
    "-Wlogical-op",
    "-Wlogical-not-parentheses",
    "-Wparentheses",
    "-Wenum-conversion",
    "-Wstring-compare",
    "-Wmemset-transposed-args",
    "-Wsizeof-pointer-memaccess",
    "-Wsizeof-array-argument",
    "-Wdangling-else",
    "-Wself-assign",
    "-Wunsequenced",
    "-Wabsolute-value",
    "-Wnonnull",
    "-Wfree-nonheap-object",
    "-Wuse-after-free",
    "-Wvla",
}

# Measured, and moved out of SIGNAL for it.  -Wcast-align is 92 findings
# and -Wpointer-sign 166 on 151 translation units.  Both are real classes -
# an alignment-increasing cast IS undefined on every strict-alignment
# architecture, and char*/u_char* confusion IS how a sign-extension bug
# starts - and both are also what this codebase does on nearly every page,
# because caddr_t is char* and every ioctl handler casts it.  258 findings
# in a gate is not a gate; it is a switch somebody turns off in week two.
# Recorded, read a subsystem at a time, not gated.

# The volume tier: a real class, read a subsystem at a time.
ADVISORY_PREFIX = ("-Wformat", "-Wcast-align", "-Wpointer-sign", "-Wconversion", "-Wsign-conversion", "-Wimplicit-int-conversion",
                   "-Wimplicit-float-conversion", "-Wshorten-64-to-32",
                   "-Wsign-compare", "-Wshadow", "-Wconstant-conversion",
                   "-Wenum-enum-conversion", "-Wenum-float-conversion",
                   "-Wbitfield-constant-conversion")

WFLAG = re.compile(r"\[(-W[A-Za-z0-9_+=-]+)\]\s*$")
# `warning: ...' with no [-Wfoo] - clang and gcc both emit a few.
PLAIN = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): "
                   r"warning: (?P<msg>.*)$", re.M)
ERRLINE = re.compile(r"^[^:\n]*:\d+:\d+: (?:fatal )?error: .*$", re.M)


@functools.lru_cache(maxsize=None)
def supported(cc: str) -> tuple[str, ...]:
    """The warning flags this compiler actually has.

    Probing rather than hardcoding, because "unknown warning option" is a
    warning and not an error: an unsupported flag does not fail the
    compile, it just silently checks nothing, which is the one outcome
    this tier is built to make impossible.
    """
    ok = []
    for w in WARNSET:
        try:
            p = subprocess.run([cc, w, "-fsyntax-only", "-xc", "/dev/null"],
                               capture_output=True, text=True, timeout=30)
        except (OSError, subprocess.SubprocessError):
            continue
        if not re.search(r"unknown warning|unrecognized command", p.stderr, re.I):
            ok.append(w)
    return tuple(ok)


def tier(flag: str) -> str:
    # SIGNAL first.  ADVISORY_PREFIX carries "-Wformat" for printf(9)'s %b,
    # and -Wformat-truncation starts with it and is nothing to do with it:
    # "specified size is 16, but format string expands to at least 17" is a
    # fact about two constants, and it found the JMicron node-name
    # truncation at sys/geom/raid/md_jmicron.c:808.  Prefix order is not a
    # classification.
    if flag in SIGNAL:
        return "signal"
    if flag.startswith(ADVISORY_PREFIX):
        return "advisory"
    return "other"


def one(job: dict) -> dict:
    cc = job["cc"]

    def run(j: dict) -> dict:
        flags = B.tu_flags(j, cc=cc)
        if "gcc" in Path(cc).name:
            # gcc has no --target=; includes.py hands out clang's spelling
            # for every compiler that is not goto-cc.  Dropping it means a
            # gcc run is only honest on the HOST architecture, which is
            # why --cc gcc is a second pass over amd64/i386 and not the
            # default.
            flags = [f for f in flags if not f.startswith("--target=")]
        warns = [*supported(cc), *OFF]
        dig = B.digest([*warns, *flags])
        cmd = [cc, "-fsyntax-only", *warns, *flags, j["src"]]
        try:
            p = subprocess.run(cmd, capture_output=True, text=True,
                               timeout=j.get("timeout", 180), cwd="/tmp")
        except subprocess.TimeoutExpired:
            return {"file": j["rel"], "status": "TIMEOUT", "findings": [],
                    "flags": dig}
        except OSError as e:
            return {"file": j["rel"], "status": "ERROR", "detail": str(e),
                    "findings": [], "flags": dig}
        if p.returncode != 0 or ERRLINE.search(p.stderr):
            m = ERRLINE.search(p.stderr)
            return {"file": j["rel"], "status": "ERROR",
                    "detail": (m.group(0) if m else p.stderr[-300:]).strip(),
                    "findings": [], "flags": dig}
        out = []
        for m in PLAIN.finditer(p.stderr):
            rel = B.rel_to_src(m.group("file"))
            # A warning in a header is a warning about a file 900
            # translation units include, and it arrives 900 times.  The
            # main file only, unless asked: a header is checked when it is
            # somebody's main file.
            if not j["headers"] and rel != j["rel"]:
                continue
            msg = m.group("msg")
            wm = WFLAG.search(msg)
            flag = wm.group(1) if wm else "-W(unnamed)"
            out.append({"where": f"{rel}:{m.group('line')}",
                        "checker": flag,
                        "msg": WFLAG.sub("", msg).strip(),
                        "tier": tier(flag)})
        return {"file": j["rel"], "status": "OK", "findings": out,
                "flags": dig}

    return B.with_retries(job, run)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    B.add_common_args(ap, "compiler_warnings.jsonl")
    ap.add_argument("--cc", default="clang",
                    help="clang (default; the only one that can target every "
                         "architecture) or gcc (host arch only, but it is "
                         f"the only one with {' '.join(sorted(GCC_ONLY))})")
    ap.add_argument("--headers", action="store_true",
                    help="keep warnings from included headers too. Off by "
                         "default: a header warning arrives once per "
                         "translation unit that includes it.")
    args = ap.parse_args()

    jobs = B.enumerate_tus(args.scope, args.limit)
    for j in jobs:
        j["timeout"] = args.timeout
        j["cc"] = args.cc
        j["headers"] = args.headers
    print(f"{len(jobs)} translation unit(s) in scope", flush=True)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not B.have(args.cc):
        B.load_resume(out, TOOL, VERSION, args.resume)
        B.notrun(jobs, out, TOOL, VERSION, INSTALL,
                 {"scopes": args.scope or B.DEFAULT_SCOPES, "cc": args.cc})
        return 1 if args.gate else 0

    have = set(supported(args.cc))
    missing = sorted(set(WARNSET) - have)
    if missing:
        # NOT a silent drop.  The whole point of the tier is that "we did
        # not check it" and "it is clean" are different answers.
        print(f"NOTE  {args.cc} has no " + " ".join(missing))
        print(f"      those classes are UNCHECKED in this run; rerun with "
              f"--cc gcc (amd64/i386 translation units only)")

    meta = {"analyzer": B.tool_version([args.cc, "--version"]),
            "cc": args.cc, "warnings": list(have),
            "unsupported": missing, "off": OFF, "headers": args.headers}
    return B.sweep(jobs, one, args, TOOL, VERSION, meta, SIGNAL,
                   quiet_findings=True)


if __name__ == "__main__":
    sys.exit(main())
