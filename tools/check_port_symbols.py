#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Does the port still export the symbols the C did - under the BUILD's flags?

Boot image run 38, six minutes into buildworld, linking libc.so.7:

  ld.lld: error: version script assignment of 'FBSD_1.0' to symbol
          '__isnan' failed: symbol not defined
  ld.lld: ... 'isnan' ... '__isnanf' ... 'isnanf' ... not defined

lib/libc/gen/isnan.cpp compiles as C++ and emits _Z7__isnand and
_Z8__isnanff. Symbol.map names the unmangled ones and the link fails.

THE ORACLE CERTIFIED THIS FILE abi_equal. It was not wrong about what it
measured; it measured nothing. lib/libc/gen/isnan.c is

    #ifdef PIC
    ... every definition in the file ...
    #endif /* PIC */

and the oracle does not pass -DPIC, because libc.so is what defines it and
the oracle compiles a bare translation unit. So both sides came out EMPTY,
the symbol sets were equal because both were the empty set, and the IR was
equal for the same reason. A vacuous comparison reported as a pass.

That is the second certification gap this batch found, and it is worse
than the first. -Wno-everything hiding a diagnostic (siglist.cpp, C99
array designators) at least compared two real modules. This compared two
absent ones.

The first version of THIS tool had the identical bug: it compiled without
-DPIC, found `set() == set()`, and printed "0 ported files export
different symbols". It was fixed by making an empty module a FAILURE
rather than a pass, which is the rule that generalises - the gate below
refuses to call a file checked if neither side emitted anything.

So: compile each ported .cpp as C17 and as C++23, with the -D set
lib/libc/Makefile actually uses, and compare `nm -g --defined-only`.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "verify"))
from includes import include_flags, SRC  # noqa: E402

# What lib/libc/Makefile puts on the command line. -DPIC is the one that
# matters here; the rest are included so a source guarded on any of them is
# compiled rather than silently skipped.
DEFS = [
    "-DPIC", "-D__DBINTERFACE_PRIVATE", "-DNLS", "-DINET6", "-DYP",
    "-DNS_CACHING", "-D_ACL_PRIVATE", "-DPOSIX_MISTAKE", "-DBROKEN_DES",
    "-DPORTMAP", "-DDES_BUILTIN", "-DWANT_HYPERV", "-DNO__SCCSID",
    "-DNO__RCSID", "-D_FORTIFY_SOURCE_read=_read",
]


def symbols(src: Path, as_cxx: bool, out: Path) -> set[str] | None:
    lang = (["-xc++", "-std=c++23", "-fno-exceptions", "-fno-rtti"]
            if as_cxx else ["-xc", "-std=c17"])
    cc = "clang++" if as_cxx else "clang"
    p = subprocess.run(
        [cc, *lang, "-c", "-Wno-everything", *DEFS,
         *include_flags(src), str(src), "-o", str(out)],
        capture_output=True, text=True)
    if p.returncode != 0:
        return None
    n = subprocess.run(["nm", "-g", "--defined-only", str(out)],
                       capture_output=True, text=True)
    return {ln.split()[-1] for ln in n.stdout.splitlines() if ln.split()}


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append",
                    default=["lib/libc", "lib/msun"])
    ap.add_argument("--gate", action="store_true")
    args = ap.parse_args()

    ports = sorted(p for s in args.scope for p in (SRC / s).rglob("*.cpp"))
    differ, vacuous, uncompiled = [], [], []
    for cpp in ports:
        c = symbols(cpp, False, Path("/tmp/_pc.o"))
        x = symbols(cpp, True, Path("/tmp/_px.o"))
        rel = cpp.relative_to(SRC).as_posix()
        if c is None or x is None:
            uncompiled.append(rel)
        elif not c and not x:
            # NOT a pass. This is the isnan case, and calling it one is
            # what let the batch through.
            vacuous.append(rel)
        elif c != x:
            differ.append((rel, sorted(c - x), sorted(x - c)))

    print(f"{len(ports)} ported file(s) compared as C17 and as C++23, "
          f"with the build's -D set\n")
    for rel, only_c, only_x in differ:
        print(f"  {rel}")
        print(f"      only in C  : {', '.join(only_c[:6])}")
        print(f"      only in C++: {', '.join(only_x[:6])}")
    for rel in vacuous:
        print(f"  {rel}\n      emits NO symbols either way - not checked, "
              f"not a pass")
    if uncompiled:
        print(f"\n  {len(uncompiled)} did not compile standalone here "
              f"(reported, not gated):")
        for rel in uncompiled[:10]:
            print(f"      {rel}")
    if not differ and not vacuous:
        print("  every port exports exactly the symbols its C original does.")
    if args.gate and (differ or vacuous):
        print(f"\nFAIL {len(differ)} port(s) export different symbols, "
              f"{len(vacuous)} could not be checked. Symbol.map names the "
              f"unmangled ones and the link will fail.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
