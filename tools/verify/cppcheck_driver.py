#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""cppcheck over hbsd/src - a second engine, written by other people.

Everything else in this verification tree is LLVM.  CBMC's C front end is
its own, but the flags, the macros and the header search that get a
translation unit as far as CBMC all come from clang, and analyze.py and
clang_tidy_driver.py are literally the same parser twice.  A defect that
clang's front end cannot represent is invisible to all three.

cppcheck is a different parser with a different value analysis, and it is
deliberately UNSOUND in the other direction: it guesses, it has
--inconclusive, and its buffer, format-string and uninitialised-value
checks find things a path engine walks past.  It also does not need the
file to compile - it works on what it could parse - which makes it the
only instrument here that says anything at all about the translation
units that are ERROR everywhere else.

That last property is a trap as much as a feature, and it is why this
driver treats cppcheck's own CRITICAL ids - syntaxError,
internalAstError, preprocessorErrorDirective, unknownMacro - as ERROR
rather than as findings.  cppcheck reports those and then CARRIES ON
checking whatever it managed to parse, so a file where it gave up halfway
reports a handful of findings and looks checked.  It is not.  That is
exactly the "a file that does not compile reads as clean" failure
analyze.py's ERROR inventory exists to prevent, one engine over.

Preprocessor note.  cppcheck defines almost nothing about the target, and
sys/sys/_types.h's `#error unsupported long size' is the first thing it
hits on every kernel file.  The fix is not a hand-written -D list - it is
to ask clang for its predefined macros for the same target includes.py
picked and hand cppcheck that as a forced include, so both engines start
from the same picture of the machine.
"""
from __future__ import annotations

import argparse
import functools
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import pbsd_breadth as B  # noqa: E402
from includes import arch_of, TRIPLE  # noqa: E402

TOOL = "cppcheck"
VERSION = 1
INSTALL = ("apt-get install -y cppcheck    # if the archive's dependency "
           "solver refuses (a broken libz3-dev pin will do it): "
           "apt-get download cppcheck libtinyxml2-10 && "
           "dpkg -i --force-depends ./libtinyxml2-10_*.deb ./cppcheck_*.deb")

# cppcheck's own severities.  `information' is about cppcheck, not about
# the code, and `debug' is about cppcheck's internals.
SEVERITIES = "warning,style,performance,portability"

# ids where cppcheck is telling you it FAILED, not what the code does.
# Treated as ERROR: the unit is unchecked, not clean.
CRITICAL = {
    "syntaxError", "internalAstError", "cppcheckError", "internalError",
    "preprocessorErrorDirective", "unknownMacro", "cppcheckLimit",
    "premium-internalError", "instantiationError", "simplifyTypedef",
}

# Suppressed, each with the reason.  The standard is analyze.py's: a check
# that fires on an idiom this tree uses on purpose is a vote on style.
SUPPRESS = {
    "unusedFunction":
        "meaningless per translation unit - every static inline in every "
        "header is 'never used' by the one .c that included it. 19 of the "
        "first 20 findings measured on sys/x86/isa/isa_dma.c were this",
    "missingInclude": "the shim directories are not a header the tree owns",
    "missingIncludeSystem": "ditto",
    "unmatchedSuppression": "about this command line, not about the code",
    "checkersReport": "cppcheck's own coverage line; read from the meta record",
    "normalCheckLevelMaxBranches":
        "cppcheck saying it stopped early - counted as a partial check in "
        "the summary, not printed once per file",
    "toomanyconfigs": "--max-configs is set on purpose",
    "variableScope": "style(9) declares at the top of the block",
    "constParameter": "const-correctness is a C++ habit",
    "constParameterPointer": "ditto - 8 of 15 on the first file measured",
    "constParameterReference": "ditto",
    "constVariable": "ditto",
    "constVariablePointer": "ditto - 4 of 15 on the first file measured",
    "constVariableReference": "ditto",
    "cstyleCast": "C",
    "useStlAlgorithm": "C++",
    "functionStatic": "C++",
    "functionConst": "C++",
    "noExplicitConstructor": "C++",
    "unusedStructMember":
        "a hardware register struct exists to describe the hardware, not to "
        "be read from",
    "unusedLabel": "style",
    "unreadVariable": "an assignment the compiler drops; -Wunused covers it",
    "redundantInitialization": "style",
    "redundantAssignment": "style; the kernel initialises then overwrites",
    "duplicateCondition": "gcc's -Wduplicated-cond is the sharper version",
    "clarifyCondition": "style",
    "clarifyCalculation": "style",
    "shadowVariable": "-Wshadow is the sharper version",
    "shadowFunction": "ditto",
    "shadowArgument": "ditto",
    "variableHidingTypedef": "style",
    "varFuncNullUB":
        "passing NULL to a variadic function. The kernel's printf(9) "
        "wrappers do it by design",
    "invalidscanf": "no scanf in the kernel",
    # -- measured over sys/geom + sys/x86: 862 of 922 findings were these --
    "comparisonError":
        "238 findings, every one `Expression (X & 0x0) == 0x8 is always "
        "false'. The mask folded to ZERO, which means cppcheck's "
        "preprocessor did not expand the constant - it is reporting its own "
        "blindness as a property of the code",
    "legacyUninitvar":
        "169 findings, every one `Uninitialized variable: _size'. _size is "
        "the local inside sys/sys/malloc.h:230's malloc() statement-"
        "expression macro. One macro, 169 findings, no defects",
    "invalidPrintfArgType_sint":
        "122 findings: %d against a u_int, and %jd against a `signed long' "
        "that IS intmax_t on LP64. printf(9) is not printf(3)",
    "invalidPrintfArgType_uint": "75 more of the same",
    "invalidPrintfArgType_float": "same",
    "invalidPrintfArgType_s": "same",
    "invalidPrintfArgType_p": "same",
    "wrongPrintfScanfArgNum":
        "24 findings, all printf(9)'s %b - the bitfield format, which takes "
        "a value AND a decoding string. cppcheck counts one",
    "funcArgNamesDifferent":
        "97 findings; a prototype and a definition naming a parameter "
        "differently is style(9)'s business, not a defect",
    "constParameterCallback": "const-correctness on a method table",
    "unsignedPositive": "`u_int >= 0' is how a bound is written defensively",
    "unsignedLessThanZero": "same",
    "knownArgument": "constant folding reported as a finding",
    "knownArgumentHiddenVariableExpression": "same",
    "knownConditionTrueFalse":
        "the kernel's configuration macros fold to constants by design; "
        "12 findings, none about behaviour",
    "multiplySizeof":
        "3 findings, all Xen's EVTCHN_2L_NR_CHANNELS, which is "
        "sizeof(xen_ulong_t) * sizeof(xen_ulong_t) * 64 on purpose",
    "duplicateValueTernary":
        "3 findings, all MIN(sizeof(a) - 1, sizeof(b)) where the two are "
        "equal. A MIN() over two constants is not a defect",
    "redundantContinue": "style",
    "ctunullpointer":
        "cppcheck's whole-program mode is not enabled here, so these are "
        "single-unit guesses about callers it cannot see",
    "compareValueOutOfTypeRangeError":
        "a bound written wider than the type, which is defensive and right",
    "ConfigurationNotChecked":
        "cppcheck saying it did not expand a macro; that is the ERROR "
        "story, and unknownMacro already carries it",
}

# A finding here is a defect, not a matter of taste.
GATE = {
    # memory
    # nullPointer is NOT in the gate, and the measurement is why: 67 of the
    # 72 on sys/geom + sys/x86 were `__pc' or `(struct pcpu*)0', which is
    # PCPU_GET()'s inline asm that cppcheck cannot model.  A 93% false
    # rate is an advisory list.  Its siblings below are the shapes that
    # do not come from that macro.
    "nullPointerDefaultArg", "nullPointerRedundantCheck",
    "nullPointerArithmetic", "nullPointerOutOfMemory",
    "arrayIndexOutOfBounds", "arrayIndexOutOfBoundsCond",
    "bufferAccessOutOfBounds", "pointerOutOfBounds", "negativeIndex",
    "negativeMemoryAllocationSize", "negativeArraySize",
    "memleak", "memleakOnRealloc", "resourceLeak", "doubleFree",
    "deallocuse", "deallocDealloc", "mismatchAllocDealloc",
    "uninitvar", "legacyUninitvar", "uninitdata", "uninitStructMember",
    "uninitMemberVar", "autoVariables", "returnDanglingLifetime",
    "danglingLifetime", "danglingTemporaryLifetime", "returnLocalVariable",
    "returnAddressOfAutoVariable", "returnReference",
    "invalidLifetime", "useClosedFile", "doubleResourceLeak",
    # arithmetic / UB
    "zerodiv", "zerodivcond", "integerOverflow", "integerOverflowCond",
    "shiftTooManyBits", "shiftTooManyBitsSigned",
    "shiftNegative", "shiftNegativeLHS", "invalidPointerCast",
    # truncLongCastAssignment and signConversion are advisory, not gated:
    # 22 findings on the sample, and compiler_warnings.py's -Wconversion
    # is the same class with a compiler's precision about promotion.
    # API misuse
    "wrongPrintfScanfArgNum", "wrongPrintfScanfParameterPositionError",
    "invalidPrintfArgType_int", "invalidPrintfArgType_uint",
    "invalidPrintfArgType_sint", "invalidPrintfArgType_float",
    "invalidPrintfArgType_s", "invalidPrintfArgType_p",
    "invalidPrintfArgType_n", "invalidFunctionArg", "invalidFunctionArgBool",
    "invalidFunctionArgStr", "sizeofwithnumericparameter",
    "sizeofwithsilentarraypointer", "sizeofDivisionMemfunc",
    "sizeofCalculation", "pointerSize", "memsetZeroBytes",
    "memsetValueOutOfRange", "memsetClassFloat", "mallocOnClassError",
    "incorrectStringBooleanError", "incorrectStringCompare",
    "staticStringCompare", "literalWithCharPtrCompare",
    "selfAssignment", "identicalInnerCondition",
    "identicalConditionAfterEarlyExit", "oppositeInnerCondition",
    "duplicateValueTernary", "duplicateExpression",
    "duplicateBreak", "redundantCopy", "commaSeparatedReturn",
    "compareBoolExpressionWithInt", "comparisonOfBoolWithBoolError",
    "assignBoolToPointer", "bitwiseOnBoolean",
    "moduloofone", "raceAfterInterlockedDecrement",
    "stringCompare", "sprintfOverlappingData", "wrongmathcall",
    "va_start_wrongParameter", "va_end_missing", "va_list_usedBeforeStarted",
}

TEMPL = "{file}:{line}:{column}: {severity}: {message} [{id}]"
LINE = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): "
                  r"(?P<sev>\w+): (?P<msg>.*?) \[(?P<id>[A-Za-z0-9_-]+)\]$",
                  re.M)


@functools.lru_cache(maxsize=None)
def predef(arch: str) -> str:
    """clang's predefined macros for ARCH, as a header cppcheck can force.

    Without this the first thing every kernel translation unit does is
    `#error unsupported long size' out of sys/sys/_types.h:56, and
    cppcheck reports that one line and nothing else - which, before the
    CRITICAL set below, read as "one finding" rather than as "not checked".
    """
    d = Path(os.environ.get("PBSD_CPPCHECK_PREDEF")
             or tempfile.mkdtemp(prefix="pbsd_cppcheck_"))
    d.mkdir(parents=True, exist_ok=True)
    out = d / f"predef_{arch}.h"
    if out.is_file():
        return str(out)
    cmd = ["clang", "-dM", "-E", "-x", "c", "/dev/null"]
    t = TRIPLE.get(arch)
    if t:
        cmd.append(f"--target={t}")
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    except (OSError, subprocess.SubprocessError):
        out.write_text("")
        return str(out)
    keep = []
    for ln in p.stdout.splitlines():
        # __has_* and the STDC feature macros are function-like or
        # compiler-internal; redefining them confuses cppcheck's own
        # preprocessor and buys nothing.
        if re.match(r"#define (__has_|__STDC_|__GNUC_|__clang)", ln):
            continue
        keep.append(ln)
    out.write_text("\n".join(keep) + "\n")
    return str(out)


def to_cppcheck(flags: list[str]) -> list[str]:
    """clang's compile line, in the flags cppcheck understands."""
    out, incdirs = [], []
    it = iter(range(len(flags)))
    i = 0
    forced = []
    while i < len(flags):
        f = flags[i]
        if f.startswith("-D"):
            out.append(f)
        elif f.startswith("-I"):
            out.append(f)
            incdirs.append(f[2:])
        elif f.startswith("-isystem"):
            d = f[len("-isystem"):] or (flags[i + 1] if i + 1 < len(flags) else "")
            if not f[len("-isystem"):]:
                i += 1
            out.append("-I" + d)
            incdirs.append(d)
        elif f == "-include":
            i += 1
            if i < len(flags):
                forced.append(flags[i])
        i += 1
    # `-include opt_global.h' names a header by the search path, and
    # cppcheck's --include= wants a path.  Resolved here rather than
    # dropped: opt_global.h is where every kernel option lives, and
    # without it half the tree preprocesses to the wrong branch.
    for name in forced:
        p = Path(name)
        if p.is_file():
            out.append(f"--include={p}")
            continue
        for d in incdirs:
            c = Path(d) / name
            if c.is_file():
                out.append(f"--include={c}")
                break
    return out


def one(job: dict) -> dict:
    def run(j: dict) -> dict:
        arch = j.get("arch") or arch_of(j["rel"])
        flags = B.tu_flags(j)
        cc_flags = to_cppcheck(flags)
        args = [j["binary"],
                f"--enable={SEVERITIES}",
                "--inconclusive",
                "--language=c" if not j["src"].endswith(".cpp") else "--language=c++",
                "--std=c11",
                f"--platform={j['platform']}",
                # One configuration - the one includes.py derived from the
                # build system.  Letting cppcheck enumerate #ifdef
                # combinations on kernel source is minutes per file and
                # answers about configurations nobody ships.
                "--max-configs=1",
                f"--include={predef(arch)}",
                f"--template={TEMPL}",
                "--error-exitcode=0",
                "-q",
                *(f"--suppress={k}" for k in sorted(SUPPRESS)),
                *cc_flags]
        if j["misra"]:
            args.append("--addon=misra")
        args.append(j["src"])
        dig = B.digest(args[1:-1])
        try:
            p = subprocess.run(args, capture_output=True, text=True,
                               timeout=j.get("timeout", 300), cwd="/tmp")
        except subprocess.TimeoutExpired:
            return {"file": j["rel"], "status": "TIMEOUT", "findings": [],
                    "flags": dig}
        except OSError as e:
            return {"file": j["rel"], "status": "ERROR", "detail": str(e),
                    "findings": [], "flags": dig}
        text = p.stdout + "\n" + p.stderr
        out, crit = [], []
        for m in LINE.finditer(text):
            ident = m.group("id")
            if ident in CRITICAL:
                crit.append(f"{B.rel_to_src(m.group('file'))}:"
                            f"{m.group('line')}: {m.group('msg')} [{ident}]")
                continue
            if ident in SUPPRESS:
                continue
            rel = B.rel_to_src(m.group("file"))
            if not j["headers"] and rel != j["rel"]:
                continue
            out.append({"where": f"{rel}:{m.group('line')}",
                        "checker": ident, "msg": m.group("msg"),
                        "sev": m.group("sev")})
        if crit:
            # cppcheck reports these and then carries on checking whatever
            # it parsed, so `out' is not empty and the file LOOKS checked.
            # It is not: some fraction of it was never seen, and there is
            # no way to know which fraction.  Unchecked, with the reason.
            return {"file": j["rel"], "status": "ERROR",
                    "detail": "; ".join(crit[:3])[:300],
                    "findings": [], "flags": dig}
        return {"file": j["rel"], "status": "OK", "findings": out,
                "flags": dig}

    return B.with_retries(job, run)


PLATFORM = {"amd64": "unix64", "aarch64": "unix64", "powerpc64": "unix64",
            "riscv64": "unix64", "i386": "unix32", "armv7": "unix32",
            "powerpc": "unix32"}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    B.add_common_args(ap, "cppcheck_results.jsonl")
    ap.add_argument("--binary", default="cppcheck")
    ap.add_argument("--misra", action="store_true",
                    help="also run the MISRA C addon. Off by default: "
                         "without --rule-texts (the standard is not "
                         "redistributable) every finding is a bare rule "
                         "number, and MISRA's advisory rules outnumber its "
                         "required ones several to one on kernel C.")
    ap.add_argument("--headers", action="store_true",
                    help="keep findings from included headers too")
    args = ap.parse_args()

    jobs = B.enumerate_tus(args.scope, args.limit)
    # One predefines directory for the whole run, shared with the workers
    # through the environment, so 151 processes do not each mkdtemp one.
    os.environ.setdefault("PBSD_CPPCHECK_PREDEF",
                          tempfile.mkdtemp(prefix="pbsd_cppcheck_"))
    for j in jobs:
        j["timeout"] = args.timeout
        j["binary"] = args.binary
        j["misra"] = args.misra
        j["headers"] = args.headers
        j["platform"] = PLATFORM.get(arch_of(j["rel"]), "unix64")
    print(f"{len(jobs)} translation unit(s) in scope", flush=True)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    if not B.have(args.binary):
        B.load_resume(out, TOOL, VERSION, args.resume)
        B.notrun(jobs, out, TOOL, VERSION, INSTALL,
                 {"scopes": args.scope or B.DEFAULT_SCOPES})
        return 1 if args.gate else 0

    for a in sorted({arch_of(j["rel"]) for j in jobs}):
        predef(a)
    meta = {"analyzer": B.tool_version([args.binary, "--version"]),
            "severities": SEVERITIES, "misra": args.misra,
            "suppressed": sorted(SUPPRESS)}
    return B.sweep(jobs, one, args, TOOL, VERSION, meta, GATE,
                   quiet_findings=True)


if __name__ == "__main__":
    sys.exit(main())
