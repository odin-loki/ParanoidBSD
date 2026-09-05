#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 15 SURGE — emit missing pbsd/ports/ stubs for inventory + progress + hand-port gaps.

Dual-world: preserves converted status in batch_progress while adding thin port stubs
alongside existing hand modules under pbsd/userland/, pbsd/net/, etc.
"""
from __future__ import annotations

import argparse
import json
import re
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
from inventory_c_sources import wave_for  # noqa: E402

HAND_PATTERNS = [
    re.compile(r"Port of ([^\s\n]+)"),
    re.compile(r"PROVENANCE:\s*([^\s\n]+)"),
    re.compile(r"Upstream:\s*([^\s\n]+)"),
    re.compile(r"@pbsd-migration-meta source=([^\s]+)"),
]


def expected_stub_path(source_posix: str, wave: str) -> Path:
    return PORTS / stub_relpath(source_posix, wave)


def inventory_missing(inv: list[dict[str, str]], *, wave_filter: str | None) -> list[dict[str, str]]:
    out: list[dict[str, str]] = []
    for row in inv:
        src = row["path"].replace("\\", "/")
        wave = row["wave"]
        if wave_filter and wave != wave_filter:
            continue
        if expected_stub_path(src, wave).exists():
            continue
        out.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": row.get("loc_estimate", "0"),
                "status": "pending",
            }
        )
    return out


def progress_missing(progress: dict, *, wave_filter: str | None) -> list[dict[str, str]]:
    out: list[dict[str, str]] = []
    for entry in progress.get("entries", []):
        src = entry.get("source", "").replace("\\", "/")
        if not src:
            continue
        wave = entry.get("wave", "wave2")
        if wave_filter and wave != wave_filter:
            continue
        stub = entry.get("stub", "")
        if stub and (ROOT / stub).exists():
            continue
        if expected_stub_path(src, wave).exists():
            continue
        out.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": str(entry.get("loc_estimate", 0)),
                "status": entry.get("status", "pending"),
            }
        )
    return out


def hand_port_missing(inv_paths: set[str], progress: dict, *, wave_filter: str | None) -> list[dict[str, str]]:
    by_source = {e["source"]: e for e in progress.get("entries", [])}
    sources: set[str] = set()
    for p in ROOT.rglob("*.cppm"):
        if "ports" in p.parts or "tests" in p.parts:
            continue
        if p.parts[0] != "pbsd":
            continue
        text = p.read_text(encoding="utf-8", errors="ignore")
        for pat in HAND_PATTERNS:
            for m in pat.finditer(text):
                src = m.group(1).strip("`\"")
                src = src.replace("\\", "/")
                if src.startswith(("hbsd/", "kde/")):
                    sources.add(src)

    out: list[dict[str, str]] = []
    for src in sorted(sources):
        if src not in inv_paths:
            continue
        wave = wave_for(src)
        if wave_filter and wave != wave_filter:
            continue
        if expected_stub_path(src, wave).exists():
            continue
        prev = by_source.get(src, {})
        out.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": str(prev.get("loc_estimate", 0)),
                "status": "converted",
            }
        )
    return out


def merge_rows(*groups: list[dict[str, str]]) -> list[dict[str, str]]:
    by_source: dict[str, dict[str, str]] = {}
    for group in groups:
        for row in group:
            src = row["source"]
            prev = by_source.get(src, {})
            by_source[src] = {**prev, **row}
    return sorted(by_source.values(), key=lambda r: (r["wave"], r["source"]))


def generate_surge(*, wave_filter: str | None = None, dry_run: bool = False) -> tuple[int, list[str]]:
    if not INVENTORY.exists():
        print("missing inventory — run tools/inventory_c_sources.py first", file=sys.stderr)
        return 0, []

    inv = load_inventory()
    inv_paths = {r["path"].replace("\\", "/") for r in inv}
    progress = load_progress()
    rows = merge_rows(
        inventory_missing(inv, wave_filter=wave_filter),
        progress_missing(progress, wave_filter=wave_filter),
        hand_port_missing(inv_paths, progress, wave_filter=wave_filter),
    )
    if not rows:
        print("no missing port stubs — inventory, progress, and hand-ports fully covered on disk")
        return 0, []

    by_source = {e["source"]: e for e in progress.get("entries", [])}
    new_entries: list[dict] = []
    samples: list[str] = []

    for row in rows:
        src = row["source"]
        wave = row["wave"]
        loc = str(row.get("loc_estimate", "0"))
        prev = by_source.get(src, {})
        status = row.get("status") or prev.get("status", "pending")
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
            "wave": "surge-b15",
            "prefixes": [
                "inventory-missing-on-disk",
                "progress-missing-stub",
                "hand-port-missing-stub",
            ],
            "generated": len(new_entries),
            "at": utc_now(),
        }
        if wave_filter:
            progress["last_batch"]["wave_filter"] = wave_filter
        save_progress(progress)

    return len(new_entries), samples


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--wave", default="", help="Optional inventory wave filter (e.g. wave2)")
    args = parser.parse_args()

    wave_filter = args.wave or None
    before = sum(1 for _ in PORTS.rglob("*.cppm")) if PORTS.exists() else 0
    written, samples = generate_surge(wave_filter=wave_filter, dry_run=args.dry_run)
    after = sum(1 for _ in PORTS.rglob("*.cppm")) if PORTS.exists() else 0
    print(f"generated {written} stub(s) under pbsd/ports/ (delta ~{after - before})")
    if samples:
        print("sample paths:")
        for s in samples:
            print(f"  {s}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
