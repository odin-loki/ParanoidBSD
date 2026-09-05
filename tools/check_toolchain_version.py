#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Keep the external toolchain and the in-tree LLVM on the same major version.

With CROSS_TOOLCHAIN the compiler comes from a package and the headers still
come from contrib/llvm-project. If those are different major versions the
build mostly works and then does not: compiler-rt builtins, the sanitizer
runtimes and libc++ ABI details are the parts that notice.

The in-tree version is recorded in one place:

    lib/clang/include/llvm/Support/VCSRevision.h
    #define LLVM_REVISION "llvmorg-21.1.8-0-g2078da43e25a"

and the toolchain file names the package. This asserts the majors match, and
prints both so a mismatch says which one to move.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

REVISION = re.compile(r'#define\s+LLVM_REVISION\s+"llvmorg-(\d+)\.')
TOOLCHAIN_VER = re.compile(r"^LLVM_VERSION\?*=\s*(\d+)\s*$", re.M)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default="hbsd/src")
    ap.add_argument("--toolchain", default="hbsd/toolchains/llvm21.mk")
    args = ap.parse_args()

    rev = Path(args.src) / "lib/clang/include/llvm/Support/VCSRevision.h"
    if not rev.is_file():
        print(f"FAIL {rev} not found - cannot tell what LLVM the tree carries.")
        return 1
    m = REVISION.search(rev.read_text(encoding="utf-8", errors="replace"))
    if not m:
        print(f"FAIL no LLVM_REVISION in {rev}.")
        return 1
    in_tree = m.group(1)

    tc = Path(args.toolchain)
    if not tc.is_file():
        print(f"FAIL {tc} not found.")
        return 1
    t = TOOLCHAIN_VER.search(tc.read_text(encoding="utf-8", errors="replace"))
    if not t:
        print(f"FAIL no LLVM_VERSION in {tc}.")
        return 1
    external = t.group(1)

    print(f"in-tree LLVM (VCSRevision.h):   {in_tree}")
    print(f"external toolchain ({tc.name}): {external}")

    if in_tree != external:
        print(f"\nFAIL major versions differ: {in_tree} vs {external}.")
        print("The build will get a long way in before compiler-rt, the")
        print("sanitizer runtimes or libc++ notice. Move one to match.")
        return 1
    print("\nOK  same major version.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
