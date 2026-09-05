# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Why does one port export a mangled name? Ask with the oracle's own flags.

25 lib/msun ports come out IR-equal and ABI-unequal, all of the
fmaximum/fminimum family:

    C only  : fmaximum
    C++ only: _Z8fmaximumdd

Two explanations have already been proposed and both were wrong, and both
were wrong the same way - a question was asked that was not the question the
oracle asks.

  1. "the __ISO_C_VISIBLE >= 2023 guard in the tree's math.h". Tested: the
     macro is 2023 under -std=c++23 exactly as under -std=c17.
  2. "the tree's math.h is on -idirafter, so the host's wins". Tested with a
     bare probe, which agreed - and the oracle also passes -I{src.parent},
     and s_fmaximum.c sits in the SAME DIRECTORY as math.h. So the tree's
     header was reachable all along and the probe was answering about a
     command line nobody runs.

So this asks with the flags the oracle actually uses, on the actual file,
and prints what each side really defines. No hypothesis: the symbol tables.

Usage:  python3 -m pbsd_passes.why_mangled [source.c ...]
"""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

from .ir_oracle import DEFINE_LINE, INTERNAL_LINKAGE, find_clang, find_clangxx
from .runner import ROOT, oracle_include_flags, stage_path_for
from .target_flags import flags_for


def _emit(compiler: str, lang: str, std: str, src: Path,
          flags: list[str]) -> tuple[str, str]:
    """Compile to LLVM IR and return (text, error)."""
    with tempfile.TemporaryDirectory(prefix="pbsd_why_") as td:
        out = Path(td) / "out.ll"
        cmd = [compiler, "-x", lang, f"-std={std}", "-O2", "-S", "-emit-llvm"]
        if lang == "c++":
            cmd += ["-fno-exceptions", "-fno-rtti"]
        cmd += [*flags, str(src), "-o", str(out)]
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0 or not out.exists():
            return "", p.stderr.strip()[:600]
        return out.read_text(), ""


def _defined(text: str) -> set[str]:
    names = set()
    for attrs, name in DEFINE_LINE.findall(text):
        if any(attrs.startswith(k) or " " + k in attrs
               for k in INTERNAL_LINKAGE):
            continue
        names.add(name)
    return names


def explain(c_src: Path) -> int:
    cxx_src = stage_path_for(c_src)
    rel = c_src.relative_to(ROOT).as_posix()
    print(f"\n== {rel}")
    if not cxx_src.exists():
        print(f"   no staged port at {cxx_src.relative_to(ROOT)}")
        return 1

    inc = oracle_include_flags(c_src)
    tgt, unresolved = flags_for(c_src)
    print(f"   include flags: {' '.join(inc)}")
    print(f"   target flags : {' '.join(tgt) or '(none)'}")
    if unresolved:
        print(f"   unresolved   : {' '.join(unresolved)}")

    c_ir, c_err = _emit(find_clang(), "c", "c17", c_src, [*tgt, *inc])
    x_ir, x_err = _emit(find_clangxx(), "c++", "c++23", cxx_src, [*tgt, *inc])
    if c_err:
        print(f"   C side failed to compile:\n     {c_err}")
    if x_err:
        print(f"   C++ side failed to compile:\n     {x_err}")
    if c_err or x_err:
        return 1

    c_syms, x_syms = _defined(c_ir), _defined(x_ir)
    print(f"   C   defines: {', '.join(sorted(c_syms)) or '-'}")
    print(f"   C++ defines: {', '.join(sorted(x_syms)) or '-'}")
    if c_syms == x_syms:
        print("   -> same symbols")
        return 0
    print(f"   -> only in C  : {', '.join(sorted(c_syms - x_syms)) or '-'}")
    print(f"   -> only in C++: {', '.join(sorted(x_syms - c_syms)) or '-'}")

    # A mangled name means the definition had no visible prototype. Say
    # whether the header the C++ side saw declares it, using the same flags
    # rather than a fresh guess at them.
    for name in sorted(c_syms - x_syms):
        with tempfile.TemporaryDirectory(prefix="pbsd_decl_") as td:
            probe = Path(td) / "decl.cc"
            probe.write_text(f'#include <math.h>\nvoid *p = (void *)&{name};\n')
            p = subprocess.run(
                [find_clangxx(), "-std=c++23", "-fsyntax-only", *inc,
                 str(probe)], capture_output=True, text=True)
        seen = "IS" if p.returncode == 0 else "is NOT"
        print(f"   <math.h> under these flags: {name} {seen} declared")
        if p.returncode != 0:
            first = (p.stderr.strip().splitlines() or [""])[0]
            print(f"     {first[:160]}")
    return 1


def main(argv: list[str]) -> int:
    if argv:
        sources = [Path(a).resolve() for a in argv]
    else:
        msun = ROOT / "hbsd" / "src" / "lib" / "msun" / "src"
        sources = [msun / "s_fmaximum.c", msun / "s_fminimum.c",
                   msun / "s_cbrt.c"]
    bad = 0
    for s in sources:
        if not s.exists():
            print(f"\n== {s}: not in the tree")
            continue
        bad += explain(s)
    return 0 if bad == 0 else 0  # diagnostic: never gates


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
