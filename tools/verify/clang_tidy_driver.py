#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""clang-tidy over hbsd/src - the same front end, a different question.

tools/verify/analyze.py runs clang's PATH-SENSITIVE analyser and then
filters its output down to sixteen checkers whose failure is a defect.
clang-tidy runs on the same AST from the same clang, and that sounds like
the same instrument twice.  It is not, for two reasons.

The first is that clang-tidy's checks are AST MATCHERS, not path
exploration.  `bugprone-suspicious-memset-usage` does not need a path to
the memset; it needs the memset.  A matcher finds the defect in code the
path engine never reaches because the enclosing function is only called
through a method table, which in this tree is most of sys/dev.

The second is narrower and sharper: analyze.py's CHECKERS list is a
WHITELIST, and everything clang's analyser says outside it is DROPPED.
`core.BitwiseShift` is not on that list.  That is the checker for

    sc->sc_output |= (1 << rcc_pins[pin].pin);   /* 1 << 2048 */

which is docs/security/UB_FINDINGS.md's rccgpio finding, and for
`(data << 16)` where data is 0x8000, which is its lge/sis finding.  Both
were found by hand.  The checker that finds them shipped in clang 16 and
this tree has never run it.

Checks are chosen the way analyze.py chose its checkers - by what a
failure MEANS.  A check that fires on an idiom the kernel uses on purpose
is not evidence, it is a vote on style, and DISABLED lists each one with
the reason.  That list is the whole design; the families are the easy
part.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbsd_breadth as B  # noqa: E402

TOOL = "clang-tidy"
# Bump when the meaning of a record changes, so --resume drops the old
# generation instead of resuming onto an answer to a different question.
VERSION = 1

INSTALL = "apt-get install -y clang-tidy-18 && update-alternatives --install " \
          "/usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-18 100"

FAMILIES = ["bugprone-*", "cert-*", "clang-analyzer-*", "misc-*",
            "performance-*"]

# Every entry is a check that FIRES ON C KERNEL CODE and should not.  A
# check that cannot fire on C at all (the C++ half of bugprone-* and
# performance-*) is not listed: it costs nothing and listing it would
# hide the ones that are a judgement call.
DISABLED = {
    # -- style, at volume -------------------------------------------------
    "bugprone-easily-swappable-parameters":
        "fires on any two adjacent int parameters; it is a naming opinion",
    "bugprone-branch-clone":
        "a switch whose arms share a body is how device tables are written",
    "bugprone-macro-parentheses":
        "thousands, in headers this tree does not own",
    "bugprone-assignment-in-if-condition":
        "`if ((e = f()) != 0)' is the kernel's error idiom, style(9) and all",
    "bugprone-switch-missing-default-case":
        "style; an exhaustive switch over an enum wants no default",
    "misc-include-cleaner":
        "include-what-you-use. 25 of the 38 findings on the first file "
        "measured, none of them about behaviour",
    "misc-header-include-cycle":
        "kernel headers include each other on purpose, with guards",
    "misc-unused-parameters":
        "every device method takes the arguments its table declares",
    "misc-const-correctness": "style, and a C++ habit",
    "misc-no-recursion": "recursion is not a defect; it is a stack question",
    "misc-definitions-in-headers":
        "`static inline' in a header is the kernel's only way to inline",
    "misc-confusable-identifiers": "homoglyph naming; noise on ASCII source",
    "misc-static-assert": "style",
    "misc-misplaced-const": "style",
    "misc-use-anonymous-namespace": "C++",
    # -- duplicates an instrument this tier already runs -------------------
    "bugprone-narrowing-conversions":
        "compiler_warnings.py runs -Wconversion, which is the same class "
        "with a compiler's precision about the promotion rules",
    "cert-dcl03-c": "alias of misc-static-assert",
    "cert-str34-c": "alias of bugprone-signed-char-misuse, kept under that name",
    "cert-dcl37-c": "alias of bugprone-reserved-identifier",
    "cert-dcl51-cpp": "alias of bugprone-reserved-identifier, C++ spelling",
    "cert-msc32-c": "alias of cert-msc51-cpp, C++",
    # -- policy, the same call analyze.py made -----------------------------
    "bugprone-reserved-identifier":
        "the kernel IS the implementation; _foo is its namespace, not a "
        "collision with it",
    "bugprone-unsafe-functions":
        "this tree calls strcpy deliberately, which is why analyze.py does "
        "not run security.insecureAPI.* either",
    "cert-msc30-c": "rand(); policy, not a defect",
    "cert-msc50-cpp": "rand(); policy, not a defect",
    "cert-dcl16-c": "the case of the L in 0L",
    "clang-analyzer-deadcode.DeadStores":
        "style, and analyze.py's docstring already argued this one",
    "clang-analyzer-optin.performance.Padding": "struct layout advice",
    # -- measured over sys/geom + sys/x86, 151 units, 562 findings ---------
    "bugprone-multi-level-implicit-pointer-conversion":
        "164 of the 562 findings on the measured sweep, and `void *' to "
        "`T **' is legal, idiomatic C - the check is a C++ hygiene rule. "
        "Four sampled, four structural",
    "bugprone-suspicious-string-compare":
        "`if (strcmp(a, b))' is how C spells `if the strings differ'. "
        "16 findings, all of them that",
    "cert-err33-c":
        "116 findings, every one an unchecked snprintf(9) into a fixed "
        "buffer. In a kernel with no libc the `check the return' rule is a "
        "policy this tree does not hold",
    "bugprone-sizeof-expression":
        "9 findings; the three sampled were Xen's own "
        "EVTCHN_2L_NR_CHANNELS = sizeof * sizeof * 64, which is deliberate "
        "and upstream. sizeof(A*) on a pointer-to-aggregate is the kernel "
        "passing a pointer size on purpose",
    # -- fires on a kernel idiom, measured ---------------------------------
    "bugprone-casting-through-void":
        "__DEVOLATILE() and KOBJMETHOD() are two macros in sys/sys, and "
        "every use of either is one of these",
    "performance-no-int-to-ptr":
        "a driver turning a bus address into a pointer is the job",
    "clang-analyzer-optin.core.EnumCastOutOfRange":
        "the kernel casts ints to enums through ioctl arguments constantly; "
        "the bound is the driver's, and onesided_index.py is the check for it",
    # -- another platform's runtime ----------------------------------------
    "clang-analyzer-osx.*": "macOS",
    "clang-analyzer-optin.osx.*": "macOS",
    "clang-analyzer-cplusplus.*": "C++",
    "clang-analyzer-optin.cplusplus.*": "C++",
    "clang-analyzer-webkit.*": "C++",
    "clang-analyzer-fuchsia.*": "another kernel",
    "clang-analyzer-optin.mpi.*": "MPI",
    "clang-analyzer-apiModeling.google.*": "gtest",
    "clang-analyzer-apiModeling.llvm.*": "LLVM's own source",
    "clang-analyzer-security.insecureAPI.*":
        "policy for a 1990s codebase, verbatim from analyze.py's docstring",
}

# The sixteen checkers analyze.py already reports.  Running them here is
# not wrong - two instruments agreeing is stronger evidence than one - but
# a merged total that counts both twice is a number nobody can read, so
# --overlap drop exists and the count is always printed.
ANALYZE_PY_OVERLAP = {
    "clang-analyzer-core.CallAndMessage", "clang-analyzer-core.DivideZero",
    "clang-analyzer-core.NonNullParamChecker",
    "clang-analyzer-core.NullDereference",
    "clang-analyzer-core.StackAddressEscape",
    "clang-analyzer-core.UndefinedBinaryOperatorResult",
    "clang-analyzer-core.VLASize",
    "clang-analyzer-core.uninitialized.ArraySubscript",
    "clang-analyzer-core.uninitialized.Assign",
    "clang-analyzer-core.uninitialized.Branch",
    "clang-analyzer-core.uninitialized.UndefReturn",
    "clang-analyzer-unix.Malloc", "clang-analyzer-unix.MallocSizeof",
    "clang-analyzer-unix.MismatchedDeallocator",
    "clang-analyzer-unix.cstring.BadSizeArg",
    "clang-analyzer-unix.cstring.NullArg",
}

# A finding here is a defect, not a matter of taste - the same standard
# analyze.py's CHECKERS list is held to.  Populated from the measured
# false-positive rate over sys/geom and sys/x86, not from the check's
# documentation.
GATE = {
    "clang-analyzer-core.BitwiseShift",
    "clang-analyzer-core.NullDereference",
    "clang-analyzer-core.DivideZero",
    "clang-analyzer-core.CallAndMessage",
    "clang-analyzer-core.uninitialized.ArraySubscript",
    "clang-analyzer-core.uninitialized.Assign",
    "clang-analyzer-core.uninitialized.Branch",
    "clang-analyzer-core.uninitialized.UndefReturn",
    "clang-analyzer-unix.Malloc",
    "clang-analyzer-unix.MallocSizeof",
    "clang-analyzer-unix.MismatchedDeallocator",
    "clang-analyzer-unix.cstring.BadSizeArg",
    "clang-analyzer-unix.cstring.NullArg",
    "clang-analyzer-valist.Uninitialized",
    "clang-analyzer-valist.CopyToSelf",
    "clang-analyzer-valist.Unterminated",
    "bugprone-suspicious-memset-usage",
    "bugprone-misplaced-pointer-arithmetic-in-alloc",
    "bugprone-misplaced-operator-in-strlen-in-alloc",
    "bugprone-string-literal-with-embedded-nul",
    "bugprone-suspicious-realloc-usage",
    "bugprone-swapped-arguments",
    "bugprone-multiple-statement-macro",
    "bugprone-incorrect-roundings",
    "bugprone-integer-division",
    "bugprone-posix-return",
    "bugprone-fold-init-type",
    "bugprone-not-null-terminated-result",
    "bugprone-undefined-memory-manipulation",
    "misc-redundant-expression",
    "misc-misleading-bidirectional",
}


def checks_arg() -> str:
    return ",".join(["-*", *FAMILIES, *(f"-{k}" for k in sorted(DISABLED))])


# clang-tidy's own error lines, which are NOT findings: the translation
# unit did not compile and therefore contributes nothing.  Matching on
# ": error:" alone is wrong, because with the repo's .clang-tidy
# (WarningsAsErrors: '*') every FINDING is printed as an error too - which
# is exactly why --config is passed explicitly below rather than letting
# clang-tidy find that file by walking up from the source.
COMPILE_ERR = re.compile(r"^[^:\n]*:\d+:\d+: error: (?!.*\[[a-z].*\]$).*$", re.M)
HARD_ERR = re.compile(r"(?:^|\n)error: |Error while processing|"
                      r"unable to handle compilation|clang-tidy: error")


def one(job: dict) -> dict:
    def run(j: dict) -> dict:
        flags = B.tu_flags(j)
        dig = B.digest(flags)
        cmd = [j["binary"], "-quiet",
               # Explicit, so the repo's own .clang-tidy - written for the
               # C++ ownership layer and carrying WarningsAsErrors: '*' -
               # is not picked up by the upward search from the source path.
               f"--config={{Checks: '{checks_arg()}', WarningsAsErrors: ''}}",
               # Diagnostics in headers are diagnostics about a header 900
               # translation units share, and they arrive 900 times.  The
               # main file only; a header gets checked when it is somebody's
               # main file.
               "--header-filter=^$",
               "--system-headers=false",
               j["src"], "--", *flags]
        try:
            p = subprocess.run(cmd, capture_output=True, text=True,
                               timeout=j.get("timeout", 180), cwd="/tmp")
        except subprocess.TimeoutExpired:
            return {"file": j["rel"], "status": "TIMEOUT", "findings": [],
                    "flags": dig}
        except OSError as e:
            return {"file": j["rel"], "status": "ERROR", "detail": str(e),
                    "findings": [], "flags": dig}
        text = p.stdout + "\n" + p.stderr
        if COMPILE_ERR.search(text) or HARD_ERR.search(p.stderr):
            m = re.search(r"error: (.{0,200})", text)
            return {"file": j["rel"], "status": "ERROR",
                    "detail": (m.group(0) if m else text[-300:]).strip(),
                    "findings": [], "flags": dig}
        out = []
        for m in B.DIAG.finditer(p.stdout):
            if m.group("sev") != "warning":
                continue
            # `[a-check,another-alias]' - the first name is the check that
            # fired, the rest are its aliases.
            check = m.group("checker").split(",")[0].strip()
            if check in ANALYZE_PY_OVERLAP and j["overlap"] == "drop":
                continue
            out.append({"where": f"{B.rel_to_src(m.group('file'))}:"
                                 f"{m.group('line')}",
                        "checker": check, "msg": m.group("msg")})
        return {"file": j["rel"], "status": "OK", "findings": out,
                "flags": dig}

    return B.with_retries(job, run)


ARGS = None


def main() -> int:
    global ARGS
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    B.add_common_args(ap, "clang_tidy_results.jsonl")
    ap.add_argument("--binary", default="clang-tidy")
    ap.add_argument("--overlap", choices=["keep", "drop"], default="keep",
                    help="what to do with the 16 checkers analyze.py already "
                         "reports. keep (default) is a second opinion; drop "
                         "is for a merged total.")
    ap.add_argument("--list-checks", action="store_true",
                    help="print the enabled set and the reason each "
                         "disabled check is disabled, then exit")
    ARGS = ap.parse_args()

    if ARGS.list_checks:
        print("families:", " ".join(FAMILIES))
        print(f"\ndisabled ({len(DISABLED)}):")
        for k, v in sorted(DISABLED.items()):
            print(f"  {k}\n      {v}")
        print(f"\ngate subset ({len(GATE)}):")
        for k in sorted(GATE):
            print(f"  {k}")
        return 0

    jobs = B.enumerate_tus(ARGS.scope, ARGS.limit)
    for j in jobs:
        j["timeout"] = ARGS.timeout
        j["binary"] = ARGS.binary
        j["overlap"] = ARGS.overlap
    print(f"{len(jobs)} translation unit(s) in scope", flush=True)

    out = Path(ARGS.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not B.have(ARGS.binary):
        B.load_resume(out, TOOL, VERSION, ARGS.resume)
        B.notrun(jobs, out, TOOL, VERSION, INSTALL,
                 {"scopes": ARGS.scope or B.DEFAULT_SCOPES})
        return 1 if ARGS.gate else 0

    meta = {"analyzer": B.tool_version([ARGS.binary, "--version"]),
            "checks": checks_arg(), "overlap": ARGS.overlap}
    return B.sweep(jobs, one, ARGS, TOOL, VERSION, meta, GATE)


if __name__ == "__main__":
    sys.exit(main())
