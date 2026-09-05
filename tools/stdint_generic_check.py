#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Do the generic <stdint.h>/<inttypes.h> headers expand to what the
per-architecture ones do?

sys/sys/_stdint_generic.h and sys/sys/_inttypes_generic.h replace five
copies each of <machine/_stdint.h> and <machine/_inttypes.h> - 2,041 lines
between them - by writing every macro in terms of a compiler predefine
instead of a number. That is only a replacement if it expands to the same
thing, so this checks, macro by macro, for all six targets.

Method:

  * a temporary include tree with the symlinks the kernel build makes -
    machine -> sys/<arch>/include, and x86, arm, arm64, powerpc, riscv ->
    sys/<family>/include - because <machine/_stdint.h> on amd64 is five
    lines that include <x86/_stdint.h>, and that name only resolves through
    one of those links;
  * clang -E, once with the architecture header included and once with the
    generic one, over a file naming every macro;
  * the two expansions are compared numerically for the limit macros -
    (-0x7f-1) and (-127 - 1) are the same value written differently - and
    textually for the printf conversions, which are string literals.

Nothing is run and no FreeBSD is needed. Only the preprocessor is asked,
and it is asked with the target's own predefines, which is where the
numbers come from in the first place.
"""
from __future__ import annotations

import argparse
import ast
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SYS = ROOT / "hbsd/src/sys"

TARGETS = {
    "amd64":   ("x86_64-unknown-freebsd15.1",  "amd64"),
    "arm64":   ("aarch64-unknown-freebsd15.1", "arm64"),
    "arm":     ("armv7-unknown-freebsd15.1-eabihf", "arm"),
    "i386":    ("i386-unknown-freebsd15.1",    "i386"),
    "powerpc": ("powerpc64-unknown-freebsd15.1", "powerpc"),
    "riscv":   ("riscv64-unknown-freebsd15.1", "riscv"),
}

PAIRS = [
    ("_stdint.h",   "sys/_stdint_generic.h",   "numeric"),
    ("_inttypes.h", "sys/_inttypes_generic.h", "string"),
]

_SUFFIX = re.compile(r"\b(0[xX][0-9a-fA-F]+|\d+)([uUlL]+)\b")


def macro_names(path: Path) -> list[str]:
    """The macros a header defines, excluding its own include guard."""
    names = []
    for line in path.read_text(errors="replace").splitlines():
        m = re.match(r"#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)", line)
        if not m:
            continue
        n = m.group(1)
        if n.startswith("_"):
            continue
        if "(" in line[m.end():m.end() + 1]:
            continue
        names.append(n)
    return sorted(set(names))


def expand(cc: str, triple: str, incs: list[Path], include: str,
           names: list[str]) -> dict[str, str]:
    """Preprocess one line per macro and pair the outputs back up."""
    src = [f"#include <{include}>"]
    for n in names:
        # The marker has to be a string literal. A bare @@INT8_MAX@@ is
        # three tokens with the macro in the middle, so the preprocessor
        # expands the label as well as the value and the output no longer
        # says which macro it came from.
        src.append(f'"@@{n}@@" {n}')
    # The visibility macros <sys/cdefs.h> would have set, and the widths
    # <sys/stdint.h> would have defined. Without them the C23 WIDTH block in
    # every <machine/_stdint.h> is skipped and the comparison silently covers
    # ten fewer macros - which is how it first reported them "missing".
    args = [cc, "-target", triple, "-E", "-P", "-nostdinc",
            "-D__ISO_C_VISIBLE=2023", "-D__BSD_VISIBLE=1",
            "-D__POSIX_VISIBLE=200809", "-D__XSI_VISIBLE=700",
            "-DINT8_WIDTH=8", "-DINT16_WIDTH=16",
            "-DINT32_WIDTH=32", "-DINT64_WIDTH=64",
            "-x", "c", "-"]
    for i in incs:
        args[-1:-1] = [f"-I{i}"]
    r = subprocess.run(args, input="\n".join(src) + "\n",
                       capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(r.stderr.strip().splitlines()[0]
                           if r.stderr.strip() else "preprocessor failed")
    out = {}
    for line in r.stdout.splitlines():
        m = re.match(r'\s*"@@([A-Za-z_][A-Za-z0-9_]*)@@"\s*(.*)', line)
        if m:
            out[m.group(1)] = m.group(2).strip()
    return out


_STR = re.compile(r'"((?:[^"\\\\]|\\\\.)*)"')


def as_string(text: str):
    """Concatenate adjacent C string literals, as the compiler would.

    FreeBSD writes PRId64 as __PRI64"d", which comes out of the
    preprocessor as "l" "d"; the generic header's __INT64_FMTd__ comes out
    as "ld". They are the same string and a textual comparison says they
    are not.
    """
    parts = _STR.findall(text)
    if not parts:
        return None
    if "".join(_STR.sub("", text).split()):
        return None  # something that is not only string literals
    return "".join(parts)


def as_number(text: str):
    """Evaluate a C integer constant expression well enough for limits."""
    t = _SUFFIX.sub(lambda m: m.group(1), text)
    t = t.replace("~", " ~ ")
    try:
        node = ast.parse(t, mode="eval")
    except SyntaxError:
        return None
    allowed = (ast.Expression, ast.BinOp, ast.UnaryOp, ast.Constant,
               ast.USub, ast.UAdd, ast.Invert, ast.Add, ast.Sub, ast.Mult,
               ast.LShift, ast.RShift, ast.BitOr, ast.BitAnd, ast.BitXor)
    for n in ast.walk(node):
        if not isinstance(n, allowed):
            return None
    try:
        return eval(compile(node, "<c>", "eval"), {"__builtins__": {}}, {})
    except Exception:
        return None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cc", default="clang")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    if shutil.which(args.cc) is None:
        print(f"SKIP {args.cc} not installed; nothing measured")
        return 0

    failures = 0
    checked = 0
    print(f"generic vs per-architecture headers, {len(TARGETS)} targets\n")
    print(f"{'header':14s} {'arch':9s} {'macros':>7s} {'same':>6s} "
          f"{'differ':>7s} {'missing':>8s}")

    tmp = Path(tempfile.mkdtemp(prefix="stdintgen."))
    for arch, (triple, mdir) in TARGETS.items():
        machine = tmp / arch
        machine.mkdir()
        (machine / "machine").symlink_to(SYS / mdir / "include")
        # sys/conf/kern.pre.mk makes these too, and <machine/_stdint.h> on
        # amd64 and i386 is a five-line file that includes <x86/_stdint.h>.
        for fam in ("x86", "arm", "arm64", "powerpc", "riscv", "i386",
                    "amd64"):
            d = SYS / fam / "include"
            if d.is_dir():
                (machine / fam).symlink_to(d)
        incs = [machine, SYS]

        for archname, genname, kind in PAIRS:
            archhdr = SYS / mdir / "include" / archname
            if not archhdr.is_file():
                print(f"{archname:14s} {arch:9s}   no such header")
                failures += 1
                continue
            names = macro_names(SYS / genname)
            try:
                a = expand(args.cc, triple, incs, f"machine/{archname}", names)
                b = expand(args.cc, triple, incs, genname, names)
            except RuntimeError as e:
                print(f"{archname:14s} {arch:9s}   preprocessor: {e}")
                failures += 1
                continue

            same = differ = missing = 0
            for n in names:
                av, bv = a.get(n), b.get(n)
                # An unexpanded name means the header does not define it.
                if av is None or av == n:
                    missing += 1
                    continue
                if bv is None or bv == n:
                    print(f"  FAIL {arch} {genname} does not define {n}")
                    differ += 1
                    failures += 1
                    continue
                if kind == "numeric":
                    na, nb = as_number(av), as_number(bv)
                    ok = na is not None and na == nb
                else:
                    ok = as_string(av) is not None and \
                        as_string(av) == as_string(bv)
                if ok:
                    same += 1
                else:
                    differ += 1
                    failures += 1
                    print(f"  FAIL {arch} {n}: machine={av!r} generic={bv!r}")
                checked += 1
            print(f"{archname:14s} {arch:9s} {len(names):7d} {same:6d} "
                  f"{differ:7d} {missing:8d}")
            if args.verbose and missing:
                absent = [n for n in names
                          if a.get(n) is None or a.get(n) == n]
                print(f"    only in the generic header: {', '.join(absent)}")

    print()
    if failures:
        print(f"{failures} macro(s) do not match. The generic header is not a")
        print("replacement until they do.")
        return 1
    print(f"{checked} macro expansions compared, all equal. The 'missing'")
    print("column counts macros the generic header defines and the")
    print("architecture's does not - run with --verbose to name them.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
