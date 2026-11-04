# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""IR-equivalence oracle — highest-leverage Tier 0 item (docs/plans/todo-passes.md)."""
from __future__ import annotations

import difflib
import hashlib
import itertools
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from .compile_db import find_clang

MAX_DIFF_LINES = 60


def find_clangxx() -> str:
    for name in ("clang++-18", "clang++"):
        p = shutil.which(name)
        if p:
            return p
    for p in ("/usr/lib/llvm-18/bin/clang++", "/usr/bin/clang++"):
        if Path(p).exists():
            return p
    return "clang++"


# `define` lines that are externally visible. internal/private linkage is a
# static and never part of the ABI, so it is excluded here on purpose.
DEFINE_LINE = re.compile(r"^define\s+(.*?)@([A-Za-z0-9_.$\\]+)\(", re.M)
INTERNAL_LINKAGE = ("internal ", "private ")


def exported_symbols(text: str) -> set[str]:
    """Names this module defines that a linker outside it can see.

    Taken from the raw IR, before normalize_ir() demangles anything - the
    point is precisely the names that demangling makes look the same.
    """
    out = set()
    for attrs, name in DEFINE_LINE.findall(text):
        if any(attrs.startswith(k) or " " + k in attrs
               for k in INTERNAL_LINKAGE):
            continue
        out.add(name)
    return out


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
    # _ZN nests, _ZL marks internal linkage (a static). Both take three
    # characters before the length-prefixed name begins.
    rest = sym[3:] if sym.startswith(("_ZN", "_ZL")) else sym[2:]
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
        # Module-level inline asm. The passes inject #include <cstdlib> into
        # every staged file, which on FreeBSD pulls in stdlib.h and its
        # .symver __qsort_r_compat, qsort_r@FBSD_1.0 directive - libc symbol
        # versioning arriving through a header, on the C++ side of every
        # comparison and the C side of none. Dropped here, with the line,
        # rather than substituted away and left as a blank.
        if line.startswith("module asm "):
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
    # `mustprogress` is on every C++ function and no C one: it is the C++11
    # forward-progress guarantee, a property of the language rather than of
    # anything the port did. Same class as noundef above. With -fno-exceptions
    # supplying `nounwind`, this is the last purely-dialectal attribute left,
    # and stripping it makes byte-identical code compare equal - verified on
    # a two-function case where neither fix alone was enough.
    text = re.sub(r"\bmustprogress\s+", "", text)
    text = re.sub(r"\bcommon\s+(?=global\b)", "", text)
    text = re.sub(r"@(_Z[A-Za-z0-9_]+)",
                  lambda m: "@" + demangle_symbol(m.group(1)), text)
    # Collapse runs of blank lines. Dropping a line above leaves the blanks
    # that surrounded it, and a difference in blank-line count is not a
    # difference in behaviour.
    text = re.sub(r"\n{2,}", "\n", text).strip()
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
    target_flags: list[str] | None = None,
) -> dict:
    """Compile both sides and compare.

    target_flags are the semantic CFLAGS the real build would use for this
    file - see target_flags.flags_for. They go on both sides, so the
    comparison stays fair while also describing the configuration that
    ships. Without them a lib/msun port is proved equivalent under
    -fno-math-errno's absence, which is not how libm is built.
    """
    include_flags = include_flags or []
    target_flags = target_flags or []
    clang = find_clang()
    clangxx = find_clangxx()
    with tempfile.TemporaryDirectory(prefix="pbsd_ir_") as td:
        td_path = Path(td)
        c_ll = td_path / "c.ll"
        cxx_ll = td_path / "cxx.ll"
        c_ok, c_err = emit_llvm(
            clang, c_src, c_ll,
            ["-x", "c", "-std=c17", *target_flags, *include_flags],
        )
        cxx_ok, cxx_err = emit_llvm(
            clangxx, cxx_src, cxx_ll,
            # -fno-exceptions is not a convenience. Without it clang gives C
            # functions `nounwind` and C++ functions nothing, because C++
            # functions may throw - so every single function read as a
            # mismatch on its attribute line. Worse, the C module carries an
            # `attributes #N = { nounwind }` group the C++ one lacks, which
            # shifts every later group number and makes identical call sites
            # differ by `#7` against `#8`.
            #
            # It is also what the code will actually be built with: kern.mk
            # compiles the port freestanding, no exceptions, no RTTI. The
            # oracle was comparing against a configuration nothing ships.
            ["-x", "c++", "-std=c++23", "-fno-exceptions", "-fno-rtti",
             *target_flags, *include_flags],
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
        c_raw = c_ll.read_text(encoding="utf-8", errors="replace")
        cxx_raw = cxx_ll.read_text(encoding="utf-8", errors="replace")

        # Linkage is checked before normalisation, because normalisation is
        # what hides it. normalize_ir() demangles @_Z13pbsd_internald back to
        # @pbsd_internal so that a body-identical port compares equal - which
        # is right for semantics and wrong for ABI. A function declared in a
        # header keeps C linkage through __BEGIN_DECLS and is fine; one that
        # is external but declared nowhere gets mangled, and libm cannot ship
        # a symbol under a new name. The oracle would have passed it.
        c_syms = exported_symbols(c_raw)
        cxx_syms = exported_symbols(cxx_raw)
        abi_equal = c_syms == cxx_syms

        c_norm = normalize_ir(c_raw)
        cxx_norm = normalize_ir(cxx_raw)
        equal = c_norm == cxx_norm
        result = {
            "equal": equal,
            "abi_equal": abi_equal,
            "target_flags": list(target_flags),
            "abi_only_in_c": sorted(c_syms - cxx_syms)[:20],
            "abi_only_in_cxx": sorted(cxx_syms - c_syms)[:20],
            "status": "ok" if equal else "mismatch",
            "c_hash": hashlib.sha256(c_norm.encode()).hexdigest()[:16],
            "cxx_hash": hashlib.sha256(cxx_norm.encode()).hexdigest()[:16],
            "c_lines": c_norm.count("\n"),
            "cxx_lines": cxx_norm.count("\n"),
        }
        if not equal:
            # A mismatch used to report two hashes and nothing else, which says
            # a port is not equivalent without saying how. That is the
            # difference between a finding and a dead end: the first FreeBSD
            # run returned 96 mismatches and there was no way to tell a real
            # semantic break from another C-vs-C++ artefact like the noundef,
            # common-linkage and name-mangling classes already normalised away.
            # Bounded so a report cannot be swamped by one pathological file.
            result["diff"] = "\n".join(
                itertools.islice(
                    difflib.unified_diff(
                        c_norm.splitlines(),
                        cxx_norm.splitlines(),
                        fromfile="c.ll",
                        tofile="cxx.ll",
                        lineterm="",
                    ),
                    MAX_DIFF_LINES,
                )
            )
        return result
