"""Differential harness for simple utilities (stdout/stderr/exit)."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

from .compile_db import find_clang
from .ir_oracle import find_clangxx


def build_and_run(
    src: Path,
    is_cxx: bool,
    inputs: list[list[str]],
    include_flags: list[str] | None = None,
) -> dict:
    include_flags = include_flags or []
    cc = find_clangxx() if is_cxx else find_clang()
    with tempfile.TemporaryDirectory(prefix="pbsd_diff_") as td:
        td_path = Path(td)
        bin_path = td_path / ("prog_cxx" if is_cxx else "prog_c")
        lang = ["-x", "c++", "-std=c++23"] if is_cxx else ["-x", "c", "-std=c17"]
        cmd = [cc, *lang, "-O0", *include_flags, "-Wno-everything", str(src), "-o", str(bin_path)]
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        if proc.returncode != 0:
            return {"built": False, "stderr": (proc.stderr or "")[-1500:], "runs": []}
        runs = []
        for argv in inputs:
            r = subprocess.run(
                [str(bin_path), *argv],
                capture_output=True,
                timeout=10,
            )
            runs.append(
                {
                    "argv": argv,
                    "exit": r.returncode,
                    "stdout": r.stdout.decode("utf-8", errors="replace"),
                    "stderr": r.stderr.decode("utf-8", errors="replace"),
                }
            )
        return {"built": True, "runs": runs}


def differential(
    c_src: Path,
    cxx_src: Path,
    inputs: list[list[str]] | None = None,
    include_flags: list[str] | None = None,
) -> dict:
    inputs = inputs or [[], ["hello"], ["-n", "x"]]
    left = build_and_run(c_src, False, inputs, include_flags)
    right = build_and_run(cxx_src, True, inputs, include_flags)
    if not left["built"] or not right["built"]:
        return {"status": "build_fail", "c": left, "cxx": right, "equal": False}
    equal = left["runs"] == right["runs"]
    return {"status": "ok" if equal else "mismatch", "equal": equal, "c": left, "cxx": right}
