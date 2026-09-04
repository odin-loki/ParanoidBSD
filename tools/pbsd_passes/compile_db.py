# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Synthetic compile_commands.json generation for HBSD scopes."""
from __future__ import annotations

import json
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def find_clang() -> str:
    for name in ("clang-18", "clang"):
        p = shutil.which(name)
        if p:
            return p
    # WSL llvm path
    for p in ("/usr/lib/llvm-18/bin/clang", "/usr/bin/clang"):
        if Path(p).exists():
            return p
    return "clang"


def default_flags(root: Path) -> list[str]:
    return [
        "-std=c17",
        "-D__FreeBSD__",
        "-D__FreeBSD_version=1500000",
        "-D__BSD_VISIBLE=1",
        f"-I{root / 'hbsd' / 'src' / 'include'}",
        f"-I{root / 'hbsd' / 'src' / 'sys'}",
        f"-I{root / 'hbsd' / 'src' / 'sys' / 'sys'}",
        f"-I{root / 'hbsd' / 'src' / 'contrib' / 'libc-vis'}",
        "-Wno-everything",
        "-ferror-limit=0",
    ]


def generate_compile_commands(
    files: list[Path],
    out: Path,
    root: Path | None = None,
) -> Path:
    root = root or ROOT
    clang = find_clang()
    flags = default_flags(root)
    entries = []
    for f in files:
        rel = f if f.is_absolute() else root / f
        cmd = [clang, "-c", *flags, str(rel)]
        entries.append(
            {
                "directory": str(root),
                "file": str(rel),
                "arguments": cmd,
            }
        )
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    return out


def coverage_report(files: list[Path], compile_commands: Path) -> dict:
    data = json.loads(compile_commands.read_text(encoding="utf-8"))
    present = {Path(e["file"]).resolve() for e in data}
    missing = [str(f) for f in files if f.resolve() not in present]
    return {
        "total_files": len(files),
        "in_db": len(files) - len(missing),
        "missing": missing[:50],
        "coverage_pct": round(100.0 * (len(files) - len(missing)) / max(1, len(files)), 2),
    }
