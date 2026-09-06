#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""What a .c -> .cpp rename makes the compiler say that it did not say before.

Boot image run 35, three minutes into buildworld, on a batch of 27 renames
the IR oracle had certified:

    lib/libc/gen/siglist.cpp:35:2: error: array designators are a C99
    extension [-Werror,-Wc99-designator]

        const char *const sys_signame[NSIG] = {
                [0] =           "Signal 0",
                [SIGHUP] =      "HUP",

C99 array designators. C++ has no such thing - C++20 added designated
initializers for STRUCTS, in declaration order, and never for arrays. clang
accepts them in C++ as an extension and warns; the FreeBSD build is
-Werror, so it stops.

THE ORACLE CANNOT SEE THIS. oracle_include_flags() opens with
-Wno-everything, deliberately: it compares IR, and a diagnostic on one side
only would be noise rather than signal. So `ir.equal` and `abi_equal` were
both true, the port was on the committable list, and it does not build. Two
true statements about a file that is not portable.

So the gate is a DIFFERENTIAL, not a fixed warning list. Compile the
original as C17 and the port as C++23 with the same warnings, and compare
the sets of warning FLAGS raised. A diagnostic that fires in both is
pre-existing and is not this port's business. A diagnostic that fires only
under C++ is exactly what the rename introduced, and is the thing -Werror
will stop the build on.

Written that way for a reason: the four lib/msun files that raise
-Wlogical-op-parentheses have been committed since run 18 and the world
builds green with them, because the C raises it too. A fixed list would
have called them regressions. The differential does not.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "verify"))
from includes import include_flags, SRC  # noqa: E402

WARN = ["-Wall", "-Wextra", "-Wno-unused-parameter", "-fsyntax-only"]
FLAG_RE = re.compile(r"\[-W([a-z0-9-]+)\]")


def flags_raised(path: Path, as_cxx: bool) -> tuple[set[str], str]:
    lang = (["-xc++", "-std=c++23", "-fno-exceptions", "-fno-rtti"]
            if as_cxx else ["-xc", "-std=c17"])
    cc = "clang++" if as_cxx else "clang"
    p = subprocess.run([cc, *lang, *WARN, *include_flags(path), str(path)],
                       capture_output=True, text=True)
    return set(FLAG_RE.findall(p.stderr)), p.stderr


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--scope", action="append",
                    default=["lib/libc", "lib/msun"])
    ap.add_argument("--gate", action="store_true",
                    help="exit 1 if any port raises a warning the C did not")
    args = ap.parse_args()

    ports = sorted(p for s in args.scope for p in (SRC / s).rglob("*.cpp"))
    findings = []
    for cpp in ports:
        cxx, cxx_err = flags_raised(cpp, True)
        # The C original: a pure rename is byte-identical, so the same file
        # compiled as C is the right comparison and needs no git archaeology.
        c, _ = flags_raised(cpp, False)
        new = cxx - c
        if new:
            lines = [l for l in cxx_err.splitlines()
                     if any(f"[-W{n}]" in l for n in new)]
            findings.append((cpp.relative_to(SRC).as_posix(), sorted(new),
                             lines[:3]))

    print(f"{len(ports)} ported .cpp compared against the same source as C\n")
    for name, new, lines in findings:
        print(f"  {name}")
        print(f"      new under C++: {', '.join('-W' + n for n in new)}")
        for l in lines:
            print(f"      {l.split(':', 3)[-1].strip()[:110]}")
    if not findings:
        print("  no port raises a diagnostic its C original does not.")
    if args.gate and findings:
        print(f"\nFAIL {len(findings)} port(s) introduce a diagnostic the C "
              f"original does not raise. The build is -Werror.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
