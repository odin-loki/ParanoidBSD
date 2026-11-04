#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""clang-tidy over the C++ tiers - a THIRD instrument, and mostly an AST one.

cxx_analyze.py is path-sensitive: it walks execution paths inside a
translation unit and says "on this path the pointer is null". clang-tidy
is, for everything outside its clang-analyzer-* bridge, an AST MATCHER -
it says "this shape is wrong" without simulating anything. The two fail
differently, which is the only reason to run both:

  cxx_analyze finds   a leak three calls deep, a use-after-free that
                      depends on a branch, a division whose zero comes
                      from another function.
  cxx_tidy finds      an operator= with no self-assignment guard, a
                      memcpy over a type with a vtable, a move
                      constructor that is not noexcept so every vector
                      growth silently COPIES. None of these needs a
                      path; all of them need a pattern.

MEASURED, 2026-09-17, clang 18.1.3: on a probe whose operator= deletes
its buffer before copying, `clang --analyze` reports nothing with its
default checkers AND nothing with -analyzer-checker=cplusplus.SelfAssignment
explicitly enabled (it is a modelling checker in 18 and emits no
diagnostic). bugprone-unhandled-self-assignment reports it. That one
measurement is the argument for this file.

HOW THE CHECK LIST WAS CHOSEN
-----------------------------
By what a failure MEANS, which is analyze.py's rule and not a taste
ranking. A check is in if a hit is a DEFECT - the program does
something other than what it says. A check is out if a hit is a
preference about how the source should look. The excluded families:

  readability-*   By name, a preference. Out.
  modernize-*     "Use auto", "use nullptr", "use trailing return".
                  A tree being ported has a house style and this is not
                  it. Out.
  llvm-*, google-*, fuchsia-*, abseil-*, android-*, altera-*, darwin-*,
  linuxkernel-*, mpi-*, objc-*, zircon-*
                  Other projects' house styles or other projects'
                  platforms. Out.
  cppcoreguidelines-*
                  Largely aliases of modernize/readability plus design
                  advice ("avoid magic numbers", "special member
                  functions"). The two members that ARE defect checks -
                  pro-type-member-init and narrowing-conversions - are
                  duplicated by bugprone or are policy; see below. Out.
  hicpp-*         Almost entirely aliases. Out, and its non-alias
                  members are reached through their originals.
  concurrency-mt-unsafe
                  A real defect class, deliberately out: it flags every
                  call to a non-reentrant libc function, and in a BSD
                  tree that is getpwnam, strerror, localtime and
                  several hundred sites that are single-threaded on
                  purpose. This is a POLICY CALL. It is written here so
                  that "we do not check MT-safety" is a decision on the
                  record and not a gap nobody noticed.

Individually excluded from families that are otherwise in - every one a
policy call, not a coverage gap, because the translation unit is still
read and still counted either way:

  bugprone-easily-swappable-parameters   design advice, enormous volume
  bugprone-narrowing-conversions         a portability tree converts
                                         widths deliberately on every
                                         page; this measures the port,
                                         not the code
  bugprone-reserved-identifier           libc and libc++ are REQUIRED
                                         to use reserved identifiers
  bugprone-macro-parentheses             style, and BSD headers are
                                         full of deliberate cases
  bugprone-branch-clone                  often deliberate; readability
  bugprone-exception-escape              KDE and FreeBSD let exceptions
                                         escape main by design
  bugprone-switch-missing-default-case   style
  bugprone-suspicious-include            style
  bugprone-argument-comment              style
  bugprone-lambda-function-name          style
  bugprone-unused-local-non-trivial-variable
                                         an RAII guard looks unused and
                                         is the entire point of RAII
  bugprone-implicit-widening-of-multiplication-result
                                         overlaps narrowing-conversions
                                         and carries the same problem
  misc-const-correctness                 style. On the nine-defect
                                         probe it produced three of the
                                         four non-analyzer hits and
                                         none of them was a defect.
  misc-include-cleaner                   include hygiene, not a defect
  misc-no-recursion                      design
  misc-unused-parameters, misc-unused-using-decls, misc-unused-alias-decls
                                         style
  cert-msc30-c, cert-msc50-cpp           rand(). Same policy as
                                         analyze.py's exclusion of
                                         security.insecureAPI.rand.
  cert-dcl*, cert-err09/58/60/61-cpp     declaration and exception
                                         house style
  bugprone-assignment-in-if-condition    MEASURED and then removed.
                                         4 of 4 hand-checked hits were
                                         the deliberate parenthesised
                                         idiom `if (!(v = f()))`, which
                                         is what the extra parentheses
                                         are for. 0 defects.

NARROWED RATHER THAN REMOVED
----------------------------
cert-err33-c produced 139 of 231 findings on the first real run - 60%
of everything the tool said - and ten of ten hand-checked were
fprintf(stderr, ...), putc() and fputs() in usr.bin/dtc. None was a
defect. Deleting the check would have lost the part that IS one: an
unchecked malloc, realloc or fread. So its CheckedFunctions option is
set to the allocation, I/O-integrity, conversion and threading
functions and nothing else. VERIFIED on a seven-call probe: the stock
list flags six including fprintf/printf/putc, the narrowed list flags
exactly malloc, realloc and fread.

The cost, stated rather than hidden: an unchecked snprintf truncation,
and an unchecked fprintf to a real file rather than to stderr, are no
longer reported by this tier at all.

DE-DUPLICATION WITH cxx_analyze.py
----------------------------------
clang-tidy runs the static analyser too, under the name
clang-analyzer-<checker>. Left alone, a use-after-free at kwin.cpp:41
would be counted once by each tool and the pair would be read as two
instruments AGREEING - which is exactly the false corroboration
report.py's agree() docstring warns about, except worse, because here
it is one instrument counted twice.

So every finding from either tool carries:

    key     "<file>:<line>:<col>:<check-name>"   exact identity
    dedup   "<file>:<line>:<col>:<check-name with the clang-analyzer-
            prefix stripped>"

Two records with the same `dedup` are the same checker on the same
token and must be merged, not summed. Two records with the same
file:line:col and DIFFERENT dedup are two instruments on one line, and
that is the corroboration worth reading. The analyzer bridge is enabled
here on purpose so that this is testable rather than theoretical - and
the check list it is given is imported from cxx_analyze.CHECKERS, so
the two cannot drift apart.

HEADER FINDINGS
---------------
clang-tidy reports ONLY the main file unless -header-filter says
otherwise, and says nothing about what it dropped. In C++ that is most
of the code: the leak is in the template in the header. This is the
same trap as plain `plist` versus `plist-multi-file`, which cost
analyze.py 13 of 255 findings in C before anybody noticed. So
-header-filter is set to the whole tree, and a finding located outside
the tree (a libstdc++ header) is kept with its absolute path rather
than dropped, so the count cannot quietly shrink.

Schema, ERROR accounting, flag sources and exit codes are
cxx_analyze.py's, imported rather than restated.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from cxx_analyze import (  # noqa: E402
    CHECKERS as ANALYZER_CHECKERS, DEFAULT_ROOT, DEFAULT_SCOPES, TMPDIR,
    _SHIM_ENV, build_shim, collect, finding, load_compile_commands,
    load_expected, prefix_of, record_path, summarise_errors,
)

# ------------------------------------------------------------ check list

# Whole families, minus the policy exclusions below.
FAMILIES = ["bugprone-*"]

# Named individually because cert-* is mostly aliases and exception
# policy; these are the memory, integer and object-lifetime ones.
CERT = [
    "cert-env33-c",      # command injection through system()
    "cert-err33-c",      # SCOPED - see CHECK_OPTIONS below
    "cert-err34-c",      # atoi-family: no way to detect failure
    "cert-flp30-c",      # floating-point loop counter
    "cert-mem57-cpp",    # over-aligned type through default operator new
    "cert-msc32-c",      # predictable PRNG seed
    "cert-oop57-cpp",    # memset/memcpy over a non-trivial type
    "cert-oop58-cpp",    # operator= that mutates its source
    "cert-str34-c",      # signed char widened to a larger integer
]

# The defect members of misc-*. The rest of the family is style.
MISC = [
    "misc-definitions-in-headers",          # ODR violation
    "misc-misplaced-const",                 # const on the wrong side
    "misc-new-delete-overloads",            # unpaired new/delete overloads
    "misc-non-copyable-objects",            # FILE*, pthread_mutex_t copied
    "misc-redundant-expression",            # `a && a`, `x | x`
    "misc-throw-by-value-catch-by-reference",  # slicing an exception
    "misc-unconventional-assign-operator",  # operator= with a wrong shape
]

# performance-* is advice, with three exceptions where the SEMANTICS
# change, not the speed. Everything else in the family is out.
PERFORMANCE = [
    # A move ctor that is not noexcept makes std::vector COPY on every
    # reallocation. That is different behaviour, not slower behaviour -
    # it runs copy constructors the author believed were never run.
    "performance-noexcept-move-constructor",
    # std::move on a const argument silently COPIES. The author wrote a
    # move and did not get one.
    "performance-move-const-arg",
    # std::move on a return value DEFEATS the move it was meant to make.
    "performance-no-automatic-move",
]

# The analyser, through clang-tidy's bridge. Imported, not restated, so
# the two tools cannot disagree about what is being checked.
ANALYZER = ["clang-analyzer-" + c for c in ANALYZER_CHECKERS]

EXCLUDE = [
    # MEASURED and then removed. On a hand-check of 20 findings from the
    # first real run (usr.bin/dtc, lib/libdevdctl, usr.sbin/ctld,
    # googletest samples), FOUR of the four assignment-in-if-condition
    # hits were the deliberate, explicitly-parenthesised BSD idiom
    #     if (!(valid = input.consume('"')))
    # which is what the extra parentheses are FOR. Zero were defects.
    "bugprone-assignment-in-if-condition",
    "bugprone-easily-swappable-parameters",
    "bugprone-narrowing-conversions",
    "bugprone-reserved-identifier",
    "bugprone-macro-parentheses",
    "bugprone-branch-clone",
    "bugprone-exception-escape",
    "bugprone-switch-missing-default-case",
    "bugprone-suspicious-include",
    "bugprone-argument-comment",
    "bugprone-lambda-function-name",
    "bugprone-unused-local-non-trivial-variable",
    "bugprone-implicit-widening-of-multiplication-result",
]

# cert-err33-c, narrowed to the functions whose result being dropped is
# a DEFECT rather than a house rule.
#
# MEASURED: on the first real run it produced 139 of 231 findings - 60%
# of everything the tool said - and a hand-check of ten of them found
# ten fprintf(stderr, ...), putc() and fputs() calls in usr.bin/dtc.
# NONE was a defect. That is precisely the class analyze.py refuses for
# security.insecureAPI.*: a 1990s codebase does not check its printf
# return and flagging every one is a rewrite proposal.
#
# Dropping the whole check would have lost the part that IS a defect -
# an unchecked malloc, realloc or fread. clang-tidy's CheckedFunctions
# option keeps it. VERIFIED on a probe with seven calls: the default
# list flags six including fprintf/printf/putc; this list flags exactly
# malloc, realloc and fread.
#
# The cost of the narrowing, stated rather than hidden: an unchecked
# snprintf truncation and an unchecked fprintf to a real file (not
# stderr) are no longer reported by this tier at all.
CHECKED_FUNCTIONS = ";".join("::" + f for f in (
    "malloc", "calloc", "realloc", "aligned_alloc",
    "fread", "fgets", "fgetc", "getc", "fgetwc",
    "strtol", "strtoll", "strtoul", "strtoull", "strtod", "strtof",
    "strtold", "strtoimax", "strtoumax",
    "signal", "setvbuf", "ftell", "fseek", "fgetpos", "fsetpos",
    "mbrtowc", "wcrtomb", "mbsrtowcs", "wcsrtombs",
    "thrd_create", "thrd_join", "mtx_init", "mtx_lock", "mtx_unlock",
    "cnd_init", "cnd_wait", "cnd_signal", "cnd_broadcast",
    "tss_create", "tss_set",
))

CHECK_OPTIONS = {
    "cert-err33-c.CheckedFunctions": CHECKED_FUNCTIONS,
    "bugprone-unused-return-value.CheckedFunctions": CHECKED_FUNCTIONS,
}
CONFIG_ARG = json.dumps({"CheckOptions": CHECK_OPTIONS})

CHECKS = ["-*"] + FAMILIES + CERT + MISC + PERFORMANCE + ANALYZER \
    + ["-" + c for c in EXCLUDE]
CHECKS_ARG = ",".join(CHECKS)

# clang-tidy's text diagnostic header. [^:\n] and not [^:], for exactly
# the reason analyze.py's DIAG comment gives: [^:]+ matches NEWLINES, so
# the file group swallows the preceding lines of source context and the
# location comes back unusable.
DIAG = re.compile(
    r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): "
    r"(?P<sev>warning|error): (?P<msg>.*?)\s*"
    r"\[(?P<check>[A-Za-z0-9_.,-]+)\]\s*$", re.M)

# A compile failure wearing a diagnostic's clothes. It is NOT a finding:
# a translation unit clang-tidy could not build reports whatever it
# managed before giving up, and that partial number read as a real one
# is the single failure mode this whole tier exists to prevent.
COMPILE_ERROR = "clang-diagnostic-error"


def tidy_version() -> str:
    try:
        out = subprocess.run(["clang-tidy", "--version"], capture_output=True,
                             text=True, timeout=20)
        for ln in (out.stdout or out.stderr).splitlines():
            if "version" in ln.lower():
                return ln.strip()
        return (out.stdout or out.stderr).splitlines()[0].strip()
    except (OSError, IndexError, subprocess.SubprocessError) as e:
        return f"unknown ({e})"


def tidy(job: dict) -> dict:
    """One translation unit through clang-tidy.

    Returns the SAME record shape cxx_analyze.py returns, so
    confidence.py and report.py cannot tell which tool wrote a file and
    do not have to.
    """
    src = Path(job["src"])
    root = Path(job["root"])
    rel = job["rel"]
    guessed = job["flagsrc"] == "guess"
    flags = list(job["flags"])
    digest = hashlib.sha256(
        "\n".join(TMPDIR.sub(r"\1<tmp>", x) for x in flags).encode()
    ).hexdigest()[:12]
    base = {"file": rel, "flags": digest, "flagsrc": job["flagsrc"],
            "tool": "clang-tidy"}

    cmd = ["clang-tidy", "--quiet", f"--checks={CHECKS_ARG}",
           f"--config={CONFIG_ARG}",
           f"--header-filter={job['header_filter']}", str(src),
           "--", *flags]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=job["timeout"], cwd=job.get("cwd", "/tmp"))
    except subprocess.TimeoutExpired:
        return {**base, "status": "TIMEOUT", "findings": []}
    except OSError as e:
        return {**base, "status": "ERROR", "detail": str(e), "findings": []}

    hits = list(DIAG.finditer(p.stdout + "\n" + p.stderr))
    errs = [m for m in hits if COMPILE_ERROR in m.group("check")]
    if errs or "Error while processing" in (p.stdout + p.stderr):
        detail = (errs[0].group(0) if errs
                  else (p.stderr.strip() or p.stdout.strip()))[-300:]
        if not job.get("retried_dialect"):
            alt = "-std=c++17" if "-std=c++20" in flags else "-std=c++20"
            nf = [x for x in flags if not x.startswith("-std=")] + [alt]
            r = tidy(dict(job, flags=nf, retried_dialect=True))
            if r["status"] != "ERROR":
                r["std"] = alt
                return r
        # The partial findings are DISCARDED. A translation unit that
        # did not build has not been checked, and half a check reported
        # as a whole one is the lie this tier is built against.
        return {**base, "status": "ERROR", "detail": detail, "findings": []}

    out, seen = [], set()
    for m in hits:
        check = m.group("check")
        if COMPILE_ERROR in check:
            continue
        # clang-tidy prints one diagnostic per alias when a check has
        # them (cert-str34-c and bugprone-signed-char-misuse are the
        # same matcher), comma-joined. The first name is the canonical
        # one; keeping all of them would triple the count of a family
        # that is mostly aliases.
        check = check.split(",")[0]
        f = Path(m.group("file"))
        where = record_path(f, root)
        line, col = int(m.group("line")), int(m.group("col"))
        # clang-tidy repeats a header diagnostic once per TU that
        # includes it, and within one run it can also repeat inside one
        # TU. Deduplicated here on the identity this schema defines.
        k = (where, line, col, check)
        if k in seen:
            continue
        seen.add(k)
        out.append(finding(where, line, col, check, m.group("msg"),
                           guessed=guessed))
    return {**base, "status": "OK", "findings": out}


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append")
    ap.add_argument("--root", default=str(DEFAULT_ROOT))
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4)))
    ap.add_argument("--timeout", type=int, default=600)
    ap.add_argument("--out", default="cxx_tidy_results.jsonl")
    ap.add_argument("--limit", type=int)
    ap.add_argument("--compile-commands", action="append", default=[])
    ap.add_argument("--no-shim", action="store_true")
    ap.add_argument("--expected", default=str(
        Path(__file__).resolve().parent / "cxx_expected_errors.json"))
    ap.add_argument("--check-errors", action="store_true",
                    help="fail if a translation unit does not compile and "
                         "is not in the inventory, or if one listed there "
                         "compiles now.")
    ap.add_argument("--list-checks", action="store_true",
                    help="print the curated list and exit. The list IS "
                         "the argument; it should be readable without "
                         "running anything.")
    args = ap.parse_args(argv)

    if args.list_checks:
        print(f"{len(CHECKS)} entries passed to --checks:")
        for c in CHECKS:
            print("   " + c)
        print("\nexcluded families: readability-*, modernize-*, llvm-*,")
        print("google-*, cppcoreguidelines-*, hicpp-*, fuchsia-*, abseil-*,")
        print("android-*, altera-*, darwin-*, linuxkernel-*, mpi-*, objc-*,")
        print("zircon-*, concurrency-mt-unsafe. See the module docstring;")
        print("every one is a policy call, and none of them hides a file.")
        return 0

    root = Path(args.root).resolve()
    if not root.is_dir():
        print(f"FAIL  --root {root} is not a directory. Nothing was run.")
        return 2

    scopes = args.scope or DEFAULT_SCOPES
    db = load_compile_commands(args.compile_commands)
    if not args.no_shim:
        os.environ[_SHIM_ENV] = build_shim(root)

    jobs, per_scope = collect(root, scopes, db, args.limit, args.timeout)
    hf = "^" + re.escape(str(root)) + "/.*"
    for j in jobs:
        j["header_filter"] = hf
    empty = [s for s, n in per_scope.items() if n == 0]
    nguess = sum(1 for j in jobs if j["flagsrc"] == "guess")

    print(f"{len(jobs)} translation unit(s) for clang-tidy across "
          f"{len(scopes)} scope(s)")
    for s in scopes:
        print(f"    {per_scope.get(s, 0):6d}  {s}"
              + ("   <-- MATCHED NO FILES" if per_scope.get(s, 0) == 0 else ""))
    print(f"  flag source: {len(jobs) - nguess} from compile_commands.json, "
          f"{nguess} GUESSED")
    print(f"  {len(CHECKS)} check selectors; --list-checks prints them")

    counts, nfind, t0 = {}, 0, time.time()
    with open(args.out, "w") as fh:
        fh.write(json.dumps({
            "_meta": True,
            "tool": "cxx_tidy.py",
            "analyzer": tidy_version(),
            "scopes": scopes,
            "scope_units": per_scope,
            "scopes_matching_nothing": empty,
            "units": len(jobs),
            "guessed_units": nguess,
            "checks": CHECKS,
            "check_options": CHECK_OPTIONS,
            "excluded_families": [
                "readability-*", "modernize-*", "llvm-*", "google-*",
                "cppcoreguidelines-*", "hicpp-*", "fuchsia-*", "abseil-*",
                "android-*", "altera-*", "darwin-*", "linuxkernel-*",
                "mpi-*", "objc-*", "zircon-*", "concurrency-mt-unsafe",
            ],
            "path_base": "hbsd/src for files under it, repository root "
                         "otherwise",
        }) + "\n")
        if jobs:
            with ProcessPoolExecutor(args.jobs) as ex:
                futs = [ex.submit(tidy, j) for j in jobs]
                for i, fut in enumerate(as_completed(futs), 1):
                    r = fut.result()
                    counts[r["status"]] = counts.get(r["status"], 0) + 1
                    nfind += len(r["findings"])
                    fh.write(json.dumps(r) + "\n")
                    for f in r["findings"]:
                        mark = " (GUESSED FLAGS)" if f.get("guessed") else ""
                        print(f"  {f['where']}  [{f['checker']}]{mark}\n"
                              f"      {f['msg'][:110]}", flush=True)
                    if i % 100 == 0 or i == len(jobs):
                        print(f"  [{i}/{len(jobs)}] "
                              f"{i / max(1e-9, time.time() - t0):.1f}/s  "
                              f"findings={nfind}  "
                              + "  ".join(f"{k}={v}"
                                          for k, v in sorted(counts.items())),
                              flush=True)

    ok = counts.get("OK", 0)
    other = ", ".join(f"{k}={v}" for k, v in sorted(counts.items())
                      if k != "OK") or "none"
    print(f"\n{ok} translation unit(s) BUILT, {len(jobs) - ok} did not "
          f"({other})")
    print(f"{nfind} finding(s) across the {ok} that built")
    for k, v in sorted(counts.items()):
        print(f"  {k:8s} {v}")
    if counts.get("ERROR"):
        summarise_errors(args.out)

    rc = 0
    if args.check_errors:
        files_x, prefixes_x = load_expected(args.expected)
        seen, erred = set(), set()
        with open(args.out, encoding="utf-8") as fh:
            for line in fh:
                r = json.loads(line)
                if r.get("_meta"):
                    continue
                seen.add(r["file"])
                if r.get("status") == "ERROR":
                    erred.add(r["file"])
        unexpected = sorted(e for e in erred if e not in files_x
                            and prefix_of(e, prefixes_x) is None)
        stale = sorted((set(files_x) & seen) - erred)
        for f in unexpected[:40]:
            print(f"FAIL  {f} does not compile and is not in the inventory")
        if len(unexpected) > 40:
            print(f"FAIL  ...and {len(unexpected) - 40} more")
        for f in stale[:40]:
            print(f"FAIL  {f} compiles now; its entry is stale ({files_x[f]})")
        if unexpected or stale:
            print(f"\n      {args.expected} is the inventory.")
            rc = 1
        elif erred:
            print(f"\nok    all {len(erred)} ERROR translation unit(s) are "
                  f"on the record")

    if empty:
        print(f"\nFAIL  {len(empty)} scope(s) matched NO FILES: "
              f"{', '.join(empty)}")
        print("      That is NOT RUN, not clean.")
        return 2
    if jobs and ok == 0:
        print(f"\nFAIL  0 translation units BUILT, {len(jobs)} ERROR.")
        print("      There is no finding count to read from this run.")
        return 2
    return rc


if __name__ == "__main__":
    sys.exit(main())
