#!/usr/bin/env python3
"""Inventory .c/.cc files under hbsd/src and kde; assign conversion waves."""
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def wave_for(rel: str) -> str:
    p = rel.replace("\\", "/")
    if p.startswith("hbsd/src/sys/kern") or p.startswith("hbsd/src/sys/vm"):
        return "wave4"
    if p.startswith("hbsd/src/sys/hardenedbsd") or "/security/" in p:
        return "wave4"
    if p.startswith("hbsd/src/sys/dev") or p.startswith("hbsd/src/sys/cam"):
        return "wave5"
    if p.startswith("hbsd/src/sys/net") or p.startswith("hbsd/src/sys/geom"):
        return "wave6"
    if "/openzfs" in p or p.startswith("hbsd/src/sys/fs"):
        return "wave6"
    if p.startswith("hbsd/src/sys/amd64") or p.startswith("hbsd/src/sys/arm64"):
        return "wave7"
    if p.startswith("hbsd/src/sys/x86") or p.startswith("hbsd/src/stand"):
        return "wave7"
    if p.startswith("hbsd/src/sys"):
        return "wave4"
    if p.startswith("hbsd/src/lib") or p.startswith("hbsd/src/bin"):
        return "wave2"
    if p.startswith("hbsd/src/sbin") or p.startswith("hbsd/src/usr.bin"):
        return "wave2"
    if p.startswith("hbsd/src/usr.sbin") or p.startswith("hbsd/src/cddl"):
        return "wave2"
    if p.startswith("kde/"):
        return "wave3"
    return "wave9"


def main() -> int:
    out = ROOT / "docs" / "migration" / "c_inventory.csv"
    out.parent.mkdir(parents=True, exist_ok=True)
    rows: list[tuple[str, str, int]] = []
    for base in (ROOT / "hbsd" / "src", ROOT / "kde"):
        if not base.exists():
            continue
        for path in base.rglob("*"):
            if path.suffix.lower() not in {".c", ".cc", ".cxx", ".cpp"}:
                continue
            if ".git" in path.parts:
                continue
            rel = path.relative_to(ROOT).as_posix()
            try:
                loc = sum(1 for _ in path.open("rb"))
            except OSError:
                loc = 0
            rows.append((rel, wave_for(rel), loc))
    rows.sort(key=lambda r: (r[1], r[0]))
    with out.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["path", "wave", "loc_estimate"])
        w.writerows(rows)
    print(f"wrote {len(rows)} rows -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
