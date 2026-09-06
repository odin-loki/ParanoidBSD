#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Does WITHOUT_MACHDEP_OPTIMIZATIONS actually turn the assembly off?

It is one line in hbsd/src.conf.pbsd and the whole of docs/ASSEMBLY.md rests
on it: select the machine-independent C instead of the hand-written
assembly. Four makefiles in the tree honour it, and they honour it four
different ways. One of them did not work.

  lib/msun/Makefile          appended COMMON_SRCS *and* ARCH_SRCS, and the
                             loop that removes the duplicate only ran in the
                             other configuration. e_fmod.c and e_fmod.S are
                             the same object name, so which one bmake built
                             was decided by suffix-rule precedence.
  lib/libc/Makefile          appends MISRCS or MDSRCS, never both. Correct.
  lib/libc/string/Makefile.inc  does not even .PATH into the architecture's
                             string directory when the option is off.
                             Correct.
  lib/libmd/Makefile         USE_ASM_SOURCES:=0 and the assembly is never
                             reached. Correct.

Nothing failed, nothing warned, and every reporter in this tree said the
option was enabled - because it was. It was enabled and inert.

So this checks two things that are not the same:

  * the collision - for lib/msun, which ARCH_SRCS entries share an object
    name with a COMMON_SRCS entry, per architecture. That is read out of the
    source lists and is a fact about the tree.
  * the mechanism - that each of the four consumers still contains the text
    that makes the option take effect, and that no fifth consumer has
    appeared without one.

Checking only the second would be circular. Checking only the first would
not notice the fix being reverted.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OPTION = "MK_MACHDEP_OPTIMIZATIONS"

# consumer -> (text that must be present, what it does)
MECHANISMS = {
    "lib/msun/Makefile": (
        "ARCH_SRCS:=  ${ARCH_SRCS:N${i}}",
        "drops the machine-dependent source that has an MI counterpart",
    ),
    "lib/libc/Makefile": (
        "SRCS+=\t${MISRCS}",
        "appends MISRCS instead of MDSRCS, never both",
    ),
    "lib/libc/string/Makefile.inc": (
        '.sinclude "${LIBC_SRCTOP}/${LIBC_ARCH}/string/Makefile.inc"',
        "includes the architecture's string sources only when the option is on",
    ),
    "lib/libmd/Makefile": (
        "USE_ASM_SOURCES:=0",
        "never reaches the assembly",
    ),
}

SRC = ROOT / "hbsd/src"
MSUN = SRC / "lib/msun"
ARCHDIRS = ["amd64", "i387", "arm", "aarch64", "powerpc", "riscv"]

_BLOCK = r"^{name}\s*\+?=\s*(.*?)(?=\n[A-Za-z#.\n]|\Z)"


def tokens(text: str, name: str, exts: tuple[str, ...]) -> set[str]:
    out: set[str] = set()
    for m in re.finditer(_BLOCK.format(name=name), text, re.S | re.M):
        for tok in m.group(1).replace("\\\n", " ").split():
            if tok.endswith(exts):
                out.add(tok)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--quiet", action="store_true")
    args = ap.parse_args()
    failures = 0

    # 1. Every consumer of the option, and only the known ones.
    found = set()
    for p in SRC.rglob("*"):
        if p.name not in ("Makefile", "Makefile.inc") and p.suffix != ".mk":
            continue
        if not p.is_file():
            continue
        try:
            text = p.read_text(errors="replace")
        except OSError:
            continue
        # A makefile COMMENT is not a consumer.
        #
        # lib/libc/amd64/string/Makefile.inc is now nothing but a comment
        # explaining why its MDSRCS is empty, and that explanation names
        # the option - so a raw substring search called it a new consumer
        # with no mechanism. The same shape as the M_NOWAIT lint reading
        # its own explanatory comment as a bug.
        code = re.sub(r"^\s*#.*$", "", text, flags=re.M)
        if OPTION not in code:
            continue
        rel = p.relative_to(SRC).as_posix()
        found.add(rel)
        if rel not in MECHANISMS:
            print(f"FAIL {rel} uses {OPTION} and is not in the table.")
            print("     A new consumer needs its off-path mechanism read and")
            print("     recorded here, because the option being set is not")
            print("     the same as the option taking effect.")
            failures += 1
            continue
        want, what = MECHANISMS[rel]
        if want not in text:
            print(f"FAIL {rel}: the mechanism is gone.")
            print(f"     expected to find {want!r}")
            print(f"     which is what {what}")
            failures += 1

    for rel in MECHANISMS:
        if rel not in found:
            print(f"FAIL {rel} no longer mentions {OPTION} at all.")
            failures += 1

    if not args.quiet:
        print(f"consumers of {OPTION}: {len(found)}")
        for rel in sorted(found):
            if rel in MECHANISMS:
                print(f"  {rel:34s} {MECHANISMS[rel][1]}")
        print()

    # 2. The collision, measured rather than assumed.
    mk = (MSUN / "Makefile").read_text(errors="replace")
    common = tokens(mk, "COMMON_SRCS", (".c",))
    if not args.quiet:
        print(f"lib/msun COMMON_SRCS: {len(common)} sources")
        print(f"{'arch':10s} {'ARCH_SRCS':>9s} {'collide':>8s}")
    total = 0
    for arch in ARCHDIRS:
        inc = MSUN / arch / "Makefile.inc"
        if not inc.is_file():
            continue
        arch_srcs = tokens(inc.read_text(errors="replace"), "ARCH_SRCS",
                           (".S", ".c"))
        collide = sorted(a for a in arch_srcs
                         if a.rsplit(".", 1)[0] + ".c" in common)
        total += len(collide)
        if not args.quiet:
            print(f"{arch:10s} {len(arch_srcs):9d} {len(collide):8d}")
    if not args.quiet:
        print()
        print(f"{total} machine-dependent sources in lib/msun share an object")
        print("name with a machine-independent one. With the option off, the")
        print("filter in lib/msun/Makefile is what stops both reaching SRCS.")

    if failures:
        print(f"\n{failures} problem(s). The option can be set and inert; that")
        print("is what this checks and it is not what a reporter checks.")
        return 1
    print("\nOK  every consumer of the option has a mechanism that takes"
          " effect.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
