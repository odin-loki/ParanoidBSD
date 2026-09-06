#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Every symbol an MDSRCS file calls must be DEFINED somewhere.

Boot image run 46 asked for `src_conf=none` - FreeBSD's own defaults, as a
bisection control - and buildworld died linking the first static binary:

    ld.lld: error: undefined symbol: __stpcpy
    >>> referenced by strcpy.c:35 (lib/libc/amd64/string/strcpy.c:35)
    ld.lld: error: undefined symbol: __memchr    (strlcat.c, strnlen.c)
    ld.lld: error: undefined symbol: __strlcpy   (strlcat.c)
    ld.lld: error: undefined symbol: __stpncpy   (strncpy.c)
    ld.lld: error: undefined symbol: __strcspn   (strsep.c)

PBSD's assembly removal deleted lib/libc/amd64/string/*.S. The C files that
CALL those routines were left behind and are still listed in MDSRCS, so
every one of them carries a prototype for a symbol that no longer exists:

    lib/libc/amd64/string/strcpy.c:30
        char *__stpcpy(char * __restrict, const char * __restrict);

A prototype is not a definition, and nothing in the tree defines these.

WHY NOTHING NOTICED

hbsd/src.conf.pbsd sets WITHOUT_MACHDEP_OPTIMIZATIONS=YES, and
lib/libc/Makefile:159 then reads

    .if empty(MDSRCS) || ${MK_MACHDEP_OPTIMIZATIONS} == no
    SRCS+=  ${MISRCS}

so MDSRCS is never selected in any build PBSD does. The tree has been
unable to build with the vendor's default options for as long as the
assembly has been gone, and the only way to find out was to ask for those
defaults - which is what run 46 did, by accident, while bisecting
something else.

check_libc_srcs.py checks that every listed source EXISTS. This checks that
what those sources call exists too, which is the half that was missing.
"""
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIBC = ROOT / "hbsd" / "src" / "lib" / "libc"

# `MDSRCS+= a.c \` continued over lines.
MDSRCS = re.compile(r"^MDSRCS\s*\+?=\s*((?:[^\n]*\\\n)*[^\n]*)", re.M)
# A call to a reserved-namespace helper.
CALL = re.compile(r"\b(__\w+)\s*\(")
# Names the compiler provides or that are not functions.
SKIP = ("__builtin", "__attribute", "__predict", "__typeof", "__has",
        "__sync", "__atomic", "__asm", "__extension", "__DECONST",
        "__containerof", "__offsetof", "__unreachable", "__CONCAT")


def defined_symbols() -> set[str]:
    """Symbols lib/libc actually DEFINES, not merely declares.

    A prototype ends in `;` and a definition in `{`, and that distinction
    is the entire point here: run 46's five undefined symbols each have a
    prototype in the very file that calls them.
    """
    out: set[str] = set()
    for f in LIBC.rglob("*.c"):
        try:
            t = f.read_text(errors="replace")
        except OSError:
            continue
        # Preprocessor lines are dropped first. FreeBSD writes a
        # conditionally-named definition as
        #
        #     char *
        #     #ifdef WEAK_STRNCPY
        #     __strncpy
        #     #else
        #     strncpy
        #     #endif
        #     (char * __restrict dst, const char * __restrict src, size_t n)
        #     {
        #
        # - lib/libc/string/strncpy.c:43, reached by powerpc64's own
        # strncpy.c, which #defines WEAK_STRNCPY and includes it. With the
        # directives in the way the name and its parameter list are not
        # adjacent and no regex over the raw text will pair them, so
        # __strncpy read as undefined and powerpc64 was reported broken
        # when it is not.
        #
        # This collects BOTH branches' names, which over-counts
        # definitions. That is the safe direction for a gate whose job is
        # to find symbols with NO definition: it can only make this quieter,
        # never noisier.
        t = re.sub(r"^\s*#.*$", "", t, flags=re.M)
        # name(...) followed by an opening brace rather than a semicolon.
        #
        # `(?:\w+\s+){0,3}` between the name and the parameter list is what
        # survives the stripped #else branch: after the directives go, the
        # text above reads `char * __strncpy strncpy (char *...) {`, and a
        # regex demanding the `(` immediately after the name pairs with
        # neither.
        for m in re.finditer(
                r"\b(__\w+)\s*(?:\w+\s+){0,3}\([^;{]*\)\s*(?:__\w+\s*)*\{",
                t):
            out.add(m.group(1))
        # __weak_reference(foo, __bar) and __strong_reference
        out.update(re.findall(r"__(?:weak|strong)_reference\s*\([^,]+,\s*(\w+)",
                              t))
    for f in LIBC.rglob("*.S"):
        try:
            t = f.read_text(errors="replace")
        except OSError:
            continue
        out.update(re.findall(
            r"\b(?:ENTRY|ASENTRY|ARCHENTRY|ARCHFUNCS|EXT)\s*\(\s*(\w+)", t))
        out.update(re.findall(r"^\s*\.globl\s+(\w+)", t, re.M))
        out.update(re.findall(r"__(?:weak|strong)_reference\s*\([^,]+,\s*(\w+)",
                              t))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero if any MDSRCS source calls a symbol "
                         "nothing defines")
    args = ap.parse_args()

    have = defined_symbols()
    bad: dict[str, list[tuple[str, list[str]]]] = collections.defaultdict(list)
    checked = 0
    for mk in sorted(LIBC.rglob("Makefile.inc")):
        m = MDSRCS.search(mk.read_text(errors="replace"))
        if not m:
            continue
        for src in [x for x in re.split(r"[\s\\]+", m.group(1))
                    if x.endswith(".c")]:
            p = mk.parent / src
            if not p.is_file():
                continue          # check_libc_srcs.py owns that case
            checked += 1
            used = {u for u in CALL.findall(p.read_text(errors="replace"))
                    if not u.startswith(SKIP)}
            gone = sorted(u for u in used if u not in have)
            if gone:
                bad[mk.parent.relative_to(ROOT).as_posix()].append((src, gone))

    n = sum(len(v) for v in bad.values())
    print(f"MDSRCS sources checked: {checked}")
    if not bad:
        print("\nevery symbol they call is defined in the tree.")
        return 0

    print(f"\n{n} source(s) call a symbol NOTHING in lib/libc defines:\n")
    for d in sorted(bad):
        print(f"  {d}")
        for src, gone in bad[d]:
            print(f"    {src:24s} {' '.join(gone)}")
    print("\nThese link only when MK_MACHDEP_OPTIMIZATIONS is YES, which is")
    print("FreeBSD's default and which hbsd/src.conf.pbsd turns off - so the")
    print("failure is invisible to every build PBSD does and immediate to")
    print("anyone who builds without that file. Boot image run 46 is how it")
    print("was found: it asked for the vendor defaults as a bisection")
    print("control and never got past libc.")
    print("\nThe fix is to drop them from MDSRCS. lib/libc/Makefile:159 then")
    print("selects MISRCS - the machine-independent C - which is what the")
    print("assembly removal was for.")
    return 1 if args.gate else 0


if __name__ == "__main__":
    sys.exit(main())
