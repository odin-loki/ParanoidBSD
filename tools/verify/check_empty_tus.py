#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""A translation unit that compiles to NOTHING reports nothing, and is OK.

`expected_errors.py` covers the file that does not compile: it reports
zero findings and is indistinguishable from a clean one, so the
inventory names every one of them and the sweep fails if the list drifts.

There is a second way to contribute nothing and be counted as clean, and
until sweep 11 nothing was watching it. `sys/conf/files` says

    netinet/tcp_ratelimit.c  optional ratelimit inet | ratelimit inet6

and `tcp_ratelimit.c:67` is `#ifdef RATELIMIT`, closed at the last line.
Compiled without that macro the whole file is a licence header and some
`#include`s. It compiles. It is OK. It has 1,064 lines of code in it and
every sweep before this one read none of them.

`includes.files_option_defines()` supplies the macro now. This checks
that it keeps doing so, from the other end: it finds every source whose
own text opens with an `#if`/`#ifdef` naming one of the options its own
`optional` clause names, with no code before it - the shape that goes
dark - and requires that the analyser's flags define that option.

The five that were genuinely empty are named as well, with the line
counts recomputed by the preprocessor rather than remembered, because
"this file has 1,064 lines the sweep can now see" is the claim, and a
claim with a number in it should be measured on the run that makes it.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import analyze  # noqa: E402
import includes  # noqa: E402

SRC = includes.SRC

# `#ifdef X', `#if defined(X)', `#if defined X' - the guard forms that
# open a file whose body is one option away from being nothing.
GUARD = re.compile(r"^\s*#\s*if(?:def)?\s+(?:defined\s*\(?\s*)?"
                   r"([A-Z_][A-Z0-9_]*)")
LINEMARK = re.compile(r'^#\s+\d+\s+"([^"]*)"')

# How much of a file may precede the guard and still count as "nothing
# before it". Licence header, #includes and a __FBSDID are all
# preprocessor or comment; five lines of real code is generous.
BODY_BEFORE = 5


def optional_tokens() -> dict[str, set[str]]:
    """source under sys/ -> the tokens its own `optional' clause names.

    Read here, out of sys/conf/files*, and NOT out of
    includes.files_option_defines() - which is the thing under test. The
    first version of this file took the candidate list from that map,
    and so could not fail: dropping RATELIMIT from the map dropped
    tcp_ratelimit.c from the candidates too, and the check reported
    "every one of them has its option defined" while the file went dark.
    A check whose input is its own answer agrees with itself.
    """
    stop = {"compile-with", "no-obj", "no-depend", "dependency", "clean",
            "warning", "before-depend", "local", "standard"}
    out: dict[str, set[str]] = {}
    for mk in sorted((SRC / "sys").rglob("files*")):
        if not mk.is_file() or mk.suffix in (".c", ".h"):
            continue
        text = mk.read_text(errors="replace").replace("\\\n", " ")
        for line in text.splitlines():
            m = re.match(r"^(\S+\.[cS])\s+optional\s+(.*)$", line)
            if not m:
                continue
            toks: set[str] = set()
            for tok in m.group(2).split():
                if tok in stop:
                    break
                if tok.startswith("!") or tok == "|":
                    continue
                toks.add(tok.upper())
            if toks:
                out.setdefault("sys/" + m.group(1), set()).update(toks)
    return out


def closes_at_end(lines: list[str], start: int) -> bool:
    """Does the guard opened at `start' close at the end of the file?

    Without this the check was wrong in the loud direction: it called
    twenty files "one #ifdef INET6 from empty" because they OPEN with a
    conditional include block and then carry eight hundred lines after
    the #endif. tcp_syncache.c is not one option from empty and saying
    so would have buried the four files that are.

    The nesting is counted from the guard to its own #endif, and what
    follows it must be nothing but blank lines, comments and further
    preprocessor lines - no code.
    """
    depth = 0
    for i in range(start, len(lines)):
        s = lines[i].strip()
        if re.match(r"^#\s*if", s):
            depth += 1
        elif re.match(r"^#\s*endif", s):
            depth -= 1
            if depth == 0:
                rest = [x.strip() for x in lines[i + 1:]]
                return not any(x and not x.startswith(("#", "/*", "*", "//"))
                               for x in rest)
    return False


def candidates() -> list[tuple[str, str]]:
    """(source, option) for every file that opens with its own option."""
    out: list[tuple[str, str]] = []
    for rel, toks in optional_tokens().items():
        p = SRC / rel
        if not p.is_file():
            continue
        lines = p.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines[:400]):
            m = GUARD.match(line)
            if not m:
                continue
            if m.group(1) not in toks:
                break
            body = [x for x in lines[:i] if x.strip()
                    and not x.strip().startswith(("#", "/*", "*", "//"))]
            if len(body) < BODY_BEFORE and closes_at_end(lines, i):
                out.append((rel, m.group(1)))
            break
    return sorted(out)


def own_lines(rel: str, with_options: bool) -> int | None:
    """Lines of THIS file's own text that survive the preprocessor.

    The headers are the same either way and would drown the difference,
    so the linemarkers decide which file each line came from.
    """
    src = SRC / rel
    arch = analyze.arch_of(rel)
    flags = list(analyze.include_flags(src, arch))
    if not with_options:
        drop = set(includes.files_option_defines().get(rel, ()))
        flags = [f for f in flags if f not in drop]
    try:
        p = subprocess.run(["clang", "-E", *analyze.lang_flags(src, rel),
                            *flags, str(src)],
                           capture_output=True, text=True, timeout=120,
                           cwd="/tmp")
    except (OSError, subprocess.SubprocessError):
        return None
    if p.returncode != 0:
        return None
    cur, n = "", 0
    for line in p.stdout.splitlines():
        m = LINEMARK.match(line)
        if m:
            cur = m.group(1)
            continue
        if line.startswith("#"):
            continue
        if cur.endswith(str(src)) and line.strip():
            n += 1
    return n


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--measure", action="store_true",
                    help="preprocess each candidate both ways and print "
                         "how much code its option turns on")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero if a candidate's option is not "
                         "defined by the analyser's flags")
    args = ap.parse_args()

    cands = candidates()
    defined = includes.files_option_defines()
    print(f"{len(defined)} sources carry a -D from their own `optional' "
          f"clause")
    print(f"{len(cands)} of them open with an #ifdef naming one of those "
          f"options, with no code before it")

    missing = [(rel, opt) for rel, opt in cands
               if f"-D{opt}" not in defined.get(rel, ())]
    for rel, opt in missing:
        print(f"  FAIL {rel}: the file is one #ifdef {opt} from empty and "
              f"the analyser does not define it")

    if args.measure:
        rows = []
        for rel, _ in cands:
            on, off = own_lines(rel, True), own_lines(rel, False)
            if on is not None and off is not None:
                rows.append((rel, off, on))
        rows.sort(key=lambda r: -(r[2] - r[1]))
        empty = [r for r in rows if r[1] == 0]
        print(f"\n{len(rows)} measured; {len(empty)} are EMPTY without "
              f"their option:")
        for rel, off, on in rows[:12]:
            print(f"   {off:6d} -> {on:6d}  (+{on - off:6d})  {rel}")
        print(f"   {sum(r[1] for r in rows):6d} -> "
              f"{sum(r[2] for r in rows):6d}  total")

    if missing:
        print(f"\n{len(missing)} source(s) would compile to nothing and "
              f"report OK.")
        return 1 if args.gate else 0
    print("\nevery one of them has its option defined.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
