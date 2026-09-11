#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""`p = realloc(p, n)': the old block, lost, with nothing left to free it.

realloc(3) returns NULL on failure and LEAVES THE OLD BLOCK ALLOCATED.
Assigning the result back over the only pointer to it therefore does two
things at once: it reports the failure, and it destroys the sole means of
recovering from it.  The caller is then holding NULL and cannot free what
it still owns.

Six of these were read one at a time in this tree before it was worth
writing the rule down:

    libfetch http_next_header()   x2   hbuf->buf = realloc(hbuf->buf, ...)
    libutil  build_iovec()             *iov = realloc(*iov, ...)
    libfigpar parse_config()      x3   directive = realloc(directive, ...)
                                       value     = realloc(value, ...)

The libfigpar three are the sharpest statement of it: nothing else in
that function held a copy, so the block was unrecoverable in principle
and not merely in practice, and the same function leaked both buffers on
all twenty-three of its returns as well.

The fix is always the same three lines -- a temporary, a test, and the
assignment only on success:

    nbuf = realloc(buf, n);
    if (nbuf == NULL)
            ...recover, with buf still valid...
    buf = nbuf;

WHAT THIS REPORTS

One rule, and it is textual on purpose: an assignment whose target is
spelled exactly the same as realloc()'s first argument, after whitespace
is normalised.  `p = realloc(p, n)', `*iov = realloc(*iov, n)',
`s->buf = realloc(s->buf, n)', and the same three inside an `if ((...) ==
NULL)'.  Nothing else.

WHAT IT DOES NOT REPORT, AND WHY

reallocf(3).  That is the whole point of reallocf: it frees the old block
itself when it cannot grow it, so `p = reallocf(p, n)' is correct and is
the idiom this rule is telling people to reach for.

realloc(9) with M_WAITOK.  The kernel's spelling takes four arguments --
addr, size, type, flags -- and a flags word naming M_WAITOK and not
M_NOWAIT is the caller saying it will sleep until the allocation
succeeds, so the NULL return this rule is about does not happen.  A
flags word that is a VARIABLE says nothing about which it will be, and
is reported.

An assignment to a DIFFERENT name.  `nbuf = realloc(buf, n)' is the shape
being recommended; whether the caller then checks nbuf is a question for
the analyser, which reports it, and not for a grep.

contrib/.  Third-party trees carry their own conventions and their own
maintainers.

A site being reported is not by itself a defect: a program that exits on
allocation failure loses nothing by losing the block.  That is what the
EXPECTED table below is for -- every site read and left alone is written
down with the reason, and --gate fails on one that is not, which is the
point: a new one has to be looked at.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parents[2] / "hbsd" / "src"

# A target is a name, optionally dereferenced, optionally with member
# accesses and constant subscripts. Deliberately narrow.
_TARGET = re.compile(
    r"(?:\*\s*)?[A-Za-z_]\w*"
    r"(?:\s*(?:->|\.)\s*[A-Za-z_]\w*|\s*\[[^\[\]]*\])*\s*$")


def _strip(text: str) -> str:
    """Comments and string literals to spaces, newlines kept."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c in "\"'":
            j, q = i + 1, c
            while j < n and text[j] != q:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def _args(text: str, open_paren: int) -> list[str] | None:
    """realloc()'s arguments, split on the commas at depth one."""
    depth, i, start, out = 0, open_paren, open_paren + 1, []
    while i < len(text):
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                out.append(text[start:i])
                return out
        elif c == "," and depth == 1:
            out.append(text[start:i])
            start = i + 1
        i += 1
    return None


def _cannot_fail(args: list[str]) -> bool:
    """realloc(9), the kernel's, with M_WAITOK: it does not return NULL.

    The kernel spelling takes four arguments -- addr, size, type, flags
    -- and a flags word naming M_WAITOK and not M_NOWAIT is the caller
    saying it is willing to sleep until the allocation succeeds.  A
    variable flags word says nothing, and is reported.
    """
    if len(args) != 4:
        return False
    flags = args[3]
    return "M_WAITOK" in flags and "M_NOWAIT" not in flags


def _norm(s: str) -> str:
    return re.sub(r"\s+", "", s)


def scan(path: Path):
    raw = path.read_text(errors="replace")
    text = _strip(raw)
    for m in re.finditer(r"\brealloc\s*\(", text):
        args = _args(text, text.index("(", m.start()))
        if not args or len(args) < 2:
            continue                 # one argument: not realloc's shape
        if _cannot_fail(args):
            continue
        first = args[0]
        # Walk back over `= ' to the assignment target.
        k = m.start()
        while k > 0 and text[k - 1] in " \t\n":
            k -= 1
        if k == 0 or text[k - 1] != "=":
            continue
        if k >= 2 and text[k - 2] in "=!<>+-*/%&|^":
            continue                 # ==, +=, ... : not a plain assignment
        j = k - 1
        stop = max(text.rfind(c, 0, j) for c in ";{}()")
        target = text[stop + 1:j]
        if not _TARGET.search(target):
            continue
        if _norm(target) != _norm(first):
            continue
        # Anchor on the ASSIGNMENT, not the realloc token: the two are
        # on different lines whenever the statement wraps, and it is the
        # assignment that has to change.
        at = stop + 1 + (len(target) - len(target.lstrip()))
        line = text.count("\n", 0, at) + 1
        src = raw.split("\n")[line - 1].strip()
        yield line, _norm(target), src


# Every site read and left alone, with the reason. A site not here fails
# --gate. The six that were NOT left alone are named in the docstring
# above and written up in docs/security/UB_FINDINGS.md.
EXPECTED: dict[str, str] = {}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append", default=[],
                    help="path prefix under hbsd/src, repeatable")
    ap.add_argument("--gate", action="store_true",
                    help="exit non-zero on a site not in EXPECTED")
    args = ap.parse_args()

    scopes = args.scope or ["lib", "bin", "sbin", "usr.bin", "usr.sbin",
                            "libexec", "sys", "include"]
    hits, unexpected = 0, 0
    for scope in scopes:
        base = SRC / scope
        if not base.exists():
            continue
        files = [base] if base.is_file() else sorted(
            p for ext in ("*.c", "*.h", "*.cpp") for p in base.rglob(ext))
        for p in files:
            rel = p.relative_to(SRC).as_posix()
            if "/contrib/" in "/" + rel or rel.startswith("contrib/"):
                continue
            for line, target, src in scan(p):
                hits += 1
                key = "%s:%d" % (rel, line)
                known = key in EXPECTED
                if not known:
                    unexpected += 1
                print("%s:%d  %s%s" % (rel, line, target,
                                       "  [expected]" if known else ""))
                print("        %s" % src[:100])

    print("\n%d site(s), %d not on the record" % (hits, unexpected),
          file=sys.stderr)
    if args.gate and unexpected:
        print("\nEach is realloc(3) returning NULL with the old block "
              "still allocated and\nnothing left pointing at it. Fix it, "
              "or write down why it is harmless.", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
