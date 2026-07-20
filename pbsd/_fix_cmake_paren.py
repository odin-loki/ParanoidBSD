#!/usr/bin/env python3
"""Collapse accidental '))' closers in pbsd CMakeLists link tails."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent


def fix_text(t: str) -> tuple[str, int]:
    n = 0
    # Lines that end with word)) before newline — keep one )
    t2, c = re.subn(r"([A-Za-z0-9_])\)\)\s*$", r"\1)", t, flags=re.M)
    n += c
    # Same mid-file before target_compile_options
    t2, c = re.subn(r"([A-Za-z0-9_])\)\)\s*\n(\s*target_compile_options)", r"\1)\n\2", t2)
    n += c
    return t2, n


def main() -> int:
    total = 0
    for rel in ("CMakeLists.txt", "bifrost/CMakeLists.txt", "userland/CMakeLists.txt"):
        p = ROOT / rel
        if not p.exists():
            continue
        t = p.read_text(encoding="utf-8", errors="replace")
        t2, n = fix_text(t)
        if n:
            p.write_text(t2, encoding="utf-8")
            print(f"{rel}: fixed={n}")
            total += n
    print(f"CMAKE_PAREN_FIXED={total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
