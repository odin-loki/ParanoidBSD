# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""IR-equivalence oracle — highest-leverage Tier 0 item (docs/plans/todo-passes.md)."""
from __future__ import annotations

import hashlib
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from .compile_db import find_clang


def find_clangxx() -> str:
    for name in ("clang++-18", "clang++"):
        p = shutil.which(name)
        if p:
            return p
    for p in ("/usr/lib/llvm-18/bin/clang++", "/usr/bin/clang++"):
        if Path(p).exists():
            return p
    return "clang++"


def demangle_symbol(sym: str) -> str:
    """Reduce an Itanium-mangled name to its bare identifier.

    Compiling as C++ mangles every non-extern-"C" function, so
    @usage_msg becomes @_Z9usage_msgv and the IR compares unequal for a port
    that is byte-for-byte semantically identical. Only the simple shapes a
    C-derived port produces are handled - a free function, or one nested in
    namespaces; anything else is left alone rather than guessed at.
    """
    if not sym.startswith("_Z"):
        return sym
    rest = sym[3:] if sym.startswith("_ZN") else sym[2:]
    parts = []
    while rest and rest[0].isdigit():
        n = 0
        while rest and rest[0].isdigit():
            n = n * 10 + int(rest[0])
            rest = rest[1:]
        if len(rest) < n:
            return sym
        parts.append(rest[:n])
        rest = rest[n:]
    if not parts:
        return sym
    return "::".join(parts)


def normalize_ir(ir: str) -> str:
    # Strip source_filename, ident, names of locals where possible, blank lines
    lines = []
    for line in ir.splitlines():
        if line.startswith("source_filename") or line.startswith("target datalayout"):
            continue
        if line.startswith(";"):
            continue
        # Drop llvm.ident metadata noise
        if "llvm.ident" in line or line.startswith("!"):
            continue
        # Canonicalize SSA names somewhat: %[[A-Za-z0-9_.]+]] → %tN later
        lines.append(line.rstrip())
    text = "\n".join(lines)
    # Drop attributes clang emits for C++ but not for C. `noundef` on a
    # return or parameter is the common one: it made every otherwise-identical
    # port read as a mismatch, which is the one thing an equivalence oracle
    # must not do. Same for the C tentative-definition `common` linkage, which
    # C++ has no equivalent of.
    text = re.sub(r"\bnoundef\s+", "", text)
    text = re.sub(r"\bcommon\s+(?=global\b)", "", text)
    text = re.sub(r"@(_Z[A-Za-z0-9_]+)",
                  lambda m: "@" + demangle_symbol(m.group(1)), text)
    # Rename %digits and %names to sequential
    counters = {"n": 0}

    def repl(m: re.Match) -> str:
        counters["n"] += 1
        return f"%t{counters['n']}"

    text = re.sub(r"%[A-Za-z0-9_.]+", repl, text)
    return text


def emit_llvm(compiler: str, src: Path, out_ll: Path, lang_flags: list[str]) -> tuple[bool, str]:
    cmd = [
        compiler,
        "-O2",
        "-emit-llvm",
        "-S",
        *lang_flags,
        "-Wno-everything",
        "-ferror-limit=0",
        str(src),
        "-o",
        str(out_ll),
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    ok = proc.returncode == 0 and out_ll.exists()
    return ok, (proc.stderr or "")[-2000:]


def compare_ir(
    c_src: Path,
    cxx_src: Path,
    include_flags: list[str] | None = None,
) -> dict:
    include_flags = include_flags or []
    clang = find_clang()
    clangxx = find_clangxx()
    with tempfile.TemporaryDirectory(prefix="pbsd_ir_") as td:
        td_path = Path(td)
        c_ll = td_path / "c.ll"
        cxx_ll = td_path / "cxx.ll"
        c_ok, c_err = emit_llvm(
            clang, c_src, c_ll, ["-x", "c", "-std=c17", *include_flags]
        )
        cxx_ok, cxx_err = emit_llvm(
            clangxx, cxx_src, cxx_ll, ["-x", "c++", "-std=c++23", *include_flags]
        )
        if not c_ok or not cxx_ok:
            return {
                "equal": False,
                "status": "compile_fail",
                "c_ok": c_ok,
                "cxx_ok": cxx_ok,
                "c_err": c_err,
                "cxx_err": cxx_err,
            }
        c_norm = normalize_ir(c_ll.read_text(encoding="utf-8", errors="replace"))
        cxx_norm = normalize_ir(cxx_ll.read_text(encoding="utf-8", errors="replace"))
        equal = c_norm == cxx_norm
        return {
            "equal": equal,
            "status": "ok" if equal else "mismatch",
            "c_hash": hashlib.sha256(c_norm.encode()).hexdigest()[:16],
            "cxx_hash": hashlib.sha256(cxx_norm.encode()).hexdigest()[:16],
            "c_lines": c_norm.count("\n"),
            "cxx_lines": cxx_norm.count("\n"),
        }
