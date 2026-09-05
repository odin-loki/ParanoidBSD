#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Find headers that will mangle a port's symbols.

A C file compiled as C++ keeps C linkage for any function that was declared
`extern "C"` first. FreeBSD's public headers do that through __BEGIN_DECLS, so
rint() survives the port untouched. Private headers written for C only often
do not, and then the definition mangles to match the declaration and the
library ships a symbol nothing can call.

That is not hypothetical: lib/msun/src/math_private.h had no __BEGIN_DECLS,
and 14 lib/msun ports were IR-equal to their originals while exporting
_Z13__kernel_coslee instead of __kernel_cosl. One pair of macros fixed all 14.
lib/libc now shows the same signature - 37 IR-equal against 34 ABI-equal - and
the header responsible is findable without running anything.

A header is reported when it declares functions at file scope and has no
linkage guard. Ranked by how many C files include it, because that is how many
ports the header can break.
"""

from __future__ import annotations

import argparse
import collections
import os
import re
import sys

GUARD = re.compile(r"__BEGIN_DECLS|extern\s+\"C\"")
# A function declaration at file scope: type, name, (args); with no body.
DECL = re.compile(
    r"^[A-Za-z_][A-Za-z0-9_ \t\*]*[ \t\*]([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{]*\)\s*"
    r"(__[a-z_]+\s*(\([^)]*\))?\s*)*;",
    re.M)
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)


# Headers PBSD has added __BEGIN_DECLS to. They are vendor files, so an
# upstream merge can drop the guard without a conflict - the same silent loss
# check_pbsd_marks.py exists for, and cheaper to assert here than to add four
# more entries there.
MUST_BE_GUARDED = [
    "lib/msun/src/math_private.h",
    "lib/libc/include/libc_private.h",
    "lib/libc/stdio/local.h",
    "lib/libc/locale/mblocal.h",
    "lib/libc/locale/xlocale_private.h",
]


def check_required(root: str) -> int:
    missing = []
    for rel in MUST_BE_GUARDED:
        path = os.path.join(root, rel)
        if not os.path.isfile(path):
            missing.append((rel, "file is gone"))
            continue
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            if not GUARD.search(fh.read()):
                missing.append((rel, "no __BEGIN_DECLS"))
    print(f"headers that must keep C linkage: {len(MUST_BE_GUARDED)}")
    for rel, why in missing:
        print(f"  FAIL {rel}: {why}")
    if missing:
        print("\nA port of anything including these will mangle its symbols,")
        print("compile, link, and ship a library nothing can call.")
        return 1
    print("all guarded.")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--scope", default="lib",
                    help="subtree to examine (default: lib)")
    ap.add_argument("--top", type=int, default=20)
    ap.add_argument("--require-guarded", action="store_true",
                    help="only check the headers PBSD has guarded; exit 1 if "
                         "one has lost its __BEGIN_DECLS")
    args = ap.parse_args()

    if args.require_guarded:
        return check_required(os.path.abspath(args.root))

    root = os.path.abspath(args.root)
    base = os.path.join(root, args.scope)
    if not os.path.isdir(base):
        print(f"no {args.scope} under {args.root}", file=sys.stderr)
        return 2

    unguarded: dict[str, int] = {}
    headers: list[str] = []
    for dirpath, dirnames, filenames in os.walk(base):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for name in filenames:
            if not name.endswith(".h"):
                continue
            path = os.path.join(dirpath, name)
            try:
                with open(path, "r", encoding="utf-8", errors="replace") as fh:
                    text = fh.read()
            except OSError:
                continue
            headers.append(path)
            body = COMMENT.sub("", text)
            decls = DECL.findall(body)
            if decls and not GUARD.search(body):
                unguarded[os.path.relpath(path, root)] = len(decls)

    # How many C files include each header, by basename - crude, but it is the
    # right order of magnitude and does not need the include path resolved.
    includers: collections.Counter[str] = collections.Counter()
    for dirpath, dirnames, filenames in os.walk(base):
        dirnames[:] = [d for d in dirnames if d != ".git"]
        for name in filenames:
            if not name.endswith(".c"):
                continue
            try:
                with open(os.path.join(dirpath, name), "r",
                          encoding="utf-8", errors="replace") as fh:
                    text = fh.read()
            except OSError:
                continue
            for inc in INCLUDE.findall(text):
                includers[os.path.basename(inc)] += 1

    rows = sorted(
        ((includers[os.path.basename(h)], n, h) for h, n in unguarded.items()),
        reverse=True)

    print(f"headers under {args.scope}: {len(headers)}")
    print(f"declaring functions with no __BEGIN_DECLS: {len(unguarded)}\n")
    print(f"{'includers':>9}  {'decls':>5}  header")
    for inc, n, h in rows[:args.top]:
        print(f"{inc:>9}  {n:>5}  {h}")
    if len(rows) > args.top:
        print(f"{'':>9}  {'':>5}  ... and {len(rows) - args.top} more")
    print("\nA port of any file including one of these keeps its own symbols")
    print("only by luck. The fix is __BEGIN_DECLS / __END_DECLS in the header.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
