#!/usr/bin/env python3
"""Find SUBDIR entries in the vendor tree that point at directories which
are not there.

FreeBSD's build walks SUBDIR lists.  A missing directory is not a warning:
bmake stops.  The tree lost usr.bin/id this way -- usr.bin/Makefile still
listed it, the directory had been eaten by a .gitignore rule during the
snapshot import, and buildworld would have died on it hours in.

Conservative by construction.  A token is only checked when it is a plain
name: anything with a '$', a wildcard, or a path separator is skipped,
because resolving those needs bmake's variable state and a guess there
would produce noise rather than findings.
"""

from __future__ import annotations

import argparse
import os
import re
import sys

SUBDIR_ASSIGN = re.compile(r"^\s*SUBDIR(?:\.[^\s=+]+)?\s*[+?:]?=\s*(.*)$")
PLAIN_NAME = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_.+-]*$")

# (Makefile relative to the tree root, SUBDIR token) -> why it may be absent.
# Empty, and worth keeping that way. It held sys/dts/Makefile's arm and
# powerpc while those architectures were dropped; they are back, so the
# exception is not a documented gap any more, just a blind spot.
EXCEPTIONS: dict[tuple[str, str], str] = {}


def subdir_tokens(makefile: str) -> list[tuple[int, str]]:
    """Return (line number, token) for every plain SUBDIR entry."""
    try:
        with open(makefile, "r", encoding="utf-8", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        return []

    found: list[tuple[int, str]] = []
    i = 0
    while i < len(lines):
        m = SUBDIR_ASSIGN.match(lines[i])
        if not m:
            i += 1
            continue
        # Walk the assignment and its backslash continuations one physical
        # line at a time, so a token is reported at the line it is on rather
        # than at the head of a list that can run twenty lines.
        value = m.group(1)
        while True:
            lineno = i + 1
            more = value.endswith("\\")
            if more:
                value = value[:-1]
            stop = False
            for token in value.split():
                if token.startswith("#"):
                    stop = True
                    break
                if PLAIN_NAME.match(token):
                    found.append((lineno, token))
            if stop or not more or i + 1 >= len(lines):
                break
            i += 1
            value = lines[i].strip()
        i += 1
    return found


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src",
                    help="tree to walk (default: hbsd/src)")
    ap.add_argument("--fail-on-missing", action="store_true",
                    help="exit 1 if anything is missing")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    if not os.path.isdir(root):
        print(f"no such tree: {args.root}", file=sys.stderr)
        return 2

    checked = 0
    excepted = 0
    missing: list[tuple[str, int, str]] = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        if "Makefile" not in filenames:
            continue
        makefile = os.path.join(dirpath, "Makefile")
        for lineno, token in subdir_tokens(makefile):
            checked += 1
            if not os.path.isdir(os.path.join(dirpath, token)):
                rel = os.path.relpath(makefile, root)
                if (rel, token) in EXCEPTIONS:
                    excepted += 1
                    continue
                missing.append((rel, lineno, token))

    print(f"SUBDIR entries checked: {checked}")
    print(f"known-absent (excepted): {excepted}")
    print(f"missing directories:    {len(missing)}")
    for rel, lineno, token in sorted(missing):
        print(f"  {rel}:{lineno}: {token}")

    if missing and args.fail_on_missing:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
