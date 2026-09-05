#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Check that every source lib/clang names actually exists.

lib/clang/libllvm/Makefile lists 2,093 sources by hand. A backend that is
enabled but whose directory is not in the tree fails there, several minutes
into buildworld, with a message about one file:

  make[4]: don't know how to make .../llvm/lib/Target/ARM/ARM.td. Stop

That happened as soon as the LLVM targets for arm, powerpc and riscv were
re-enabled, because the snapshot had pruned every backend except AArch64 and
X86 from contrib/llvm-project along with the architectures themselves.

Resolution follows lib/clang/llvm.build.mk: each Makefile sets SRCDIR, the
.PATH is ${LLVM_BASE}/${SRCDIR}, and SRCS entries are relative to that.
Makefiles that build SRCDIR from a variable are skipped and counted, rather
than guessed at.
"""

from __future__ import annotations

import argparse
import os
import re
import sys

SRCDIR = re.compile(r"^SRCDIR\s*[:?]?=\s*(\S+)\s*$")
SRCS = re.compile(r"^SRCS(?:_[A-Z0-9_]+)?\s*[+:?]?=\s*(\S+)\s*$")
LLVM_BASE = os.path.join("contrib", "llvm-project")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("root", nargs="?", default="hbsd/src")
    ap.add_argument("--fail-on-missing", action="store_true")
    args = ap.parse_args()

    root = os.path.abspath(args.root)
    base = os.path.join(root, LLVM_BASE)
    if not os.path.isdir(base):
        print(f"no llvm-project under {args.root}", file=sys.stderr)
        return 2

    checked = 0
    skipped = 0
    missing: list[tuple[str, int, str]] = []
    for sub in ("lib/clang", "usr.bin/clang", "lib/libclang_rt"):
        for dirpath, dirnames, filenames in os.walk(os.path.join(root, sub)):
            dirnames[:] = [d for d in dirnames if d != ".git"]
            if "Makefile" not in filenames:
                continue
            path = os.path.join(dirpath, "Makefile")
            with open(path, "r", encoding="utf-8", errors="replace") as fh:
                lines = fh.read().splitlines()

            srcdir = None
            for line in lines:
                m = SRCDIR.match(line)
                if m:
                    srcdir = m.group(1)
                    break
            if srcdir is None:
                continue
            if "$" in srcdir:
                skipped += 1
                continue

            rel = os.path.relpath(path, root)
            for lineno, line in enumerate(lines, 1):
                m = SRCS.match(line)
                if not m:
                    continue
                entry = m.group(1)
                if "$" in entry or "/" not in entry:
                    continue
                checked += 1
                if not os.path.exists(os.path.join(base, srcdir, entry)):
                    missing.append((rel, lineno, f"{srcdir}/{entry}"))

    print(f"clang/llvm sources checked: {checked}")
    print(f"makefiles skipped (computed SRCDIR): {skipped}")
    print(f"missing: {len(missing)}")
    for rel, lineno, what in missing[:40]:
        print(f"  {rel}:{lineno}: {what}")
    if len(missing) > 40:
        print(f"  ... and {len(missing) - 40} more")

    if missing and args.fail_on_missing:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
