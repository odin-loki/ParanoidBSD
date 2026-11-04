#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 13 SURGE — emit missing pbsd/ports/ stubs for inventory rows without on-disk files.

Dual-world: preserves converted status in batch_progress while adding thin port stubs
alongside existing hand modules under pbsd/userland/, pbsd/net/, etc.
"""
from __future__ import annotations

import csv
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))

from convert_c_batch import (  # noqa: E402
    INVENTORY,
    PORTS,
    PROGRESS,
    load_inventory,
    load_progress,
    merge_entries,
    module_name_for,
    recompute_wave_stats,
    render_meta,
    render_stub,
    save_progress,
    stub_relpath,
    utc_now,
)


def expected_stub_path(source_posix: str, wave: str) -> Path:
    return PORTS / stub_relpath(source_posix, wave)


def missing_rows(inv: list[dict[str, str]], progress: dict) -> list[dict[str, str]]:
    by_source = {e["source"]: e for e in progress.get("entries", [])}
    out: list[dict[str, str]] = []
    for row in inv:
        src = row["path"].replace("\\", "/")
        wave = row["wave"]
        if expected_stub_path(src, wave).exists():
            continue
        prev = by_source.get(src, {})
        out.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": row.get("loc_estimate", "0"),
                "status": prev.get("status", "pending"),
            }
        )
    return out


def generate_surge(*, dry_run: bool = False) -> tuple[int, list[str]]:
    if not INVENTORY.exists():
        print("missing inventory — run tools/inventory_c_sources.py first", file=sys.stderr)
        return 0, []

    inv = load_inventory()
    progress = load_progress()
    rows = missing_rows(inv, progress)
    if not rows:
        print("no missing port stubs — inventory fully covered on disk")
        return 0, []

    new_entries: list[dict] = []
    samples: list[str] = []

    for row in rows:
        src = row["source"]
        wave = row["wave"]
        loc = str(row.get("loc_estimate", "0"))
        status = row.get("status", "pending")
        stub_status = "converted" if status == "converted" else "stubbed"
        if status not in {"converted", "stubbed"}:
            status = "stubbed"

        rel_stub = stub_relpath(src, wave)
        stub_path = PORTS / rel_stub
        meta_path = stub_path.with_suffix(".cppm.meta.json")
        rel_posix = (Path("pbsd/ports") / rel_stub).as_posix()

        if not dry_run:
            stub_path.parent.mkdir(parents=True, exist_ok=True)
            stub_path.write_text(render_stub(src, wave, loc, stub_status), encoding="utf-8")
            meta = render_meta(src, wave, loc, stub_status, Path("pbsd/ports") / rel_stub)
            meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")

        new_entries.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": int(loc) if loc.isdigit() else 0,
                "status": status,
                "stub": rel_posix,
                "module": module_name_for(src, wave),
            }
        )
        if len(samples) < 5:
            samples.append(rel_posix)

    if not dry_run:
        progress["entries"] = merge_entries(progress.get("entries", []), new_entries)
        progress["waves"] = recompute_wave_stats(progress["entries"])
        progress["last_batch"] = {
            "wave": "surge",
            "prefixes": ["inventory-missing-on-disk"],
            "generated": len(new_entries),
            "at": utc_now(),
        }
        save_progress(progress)

    return len(new_entries), samples


def main() -> int:
    dry_run = "--dry-run" in sys.argv
    before = sum(1 for _ in PORTS.rglob("*.cppm")) if PORTS.exists() else 0
    written, samples = generate_surge(dry_run=dry_run)
    after = sum(1 for _ in PORTS.rglob("*.cppm")) if PORTS.exists() else 0
    print(f"generated {written} stub(s) under pbsd/ports/ (delta ~{after - before})")
    if samples:
        print("sample paths:")
        for s in samples:
            print(f"  {s}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
