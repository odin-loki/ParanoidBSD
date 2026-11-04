#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Mark inventory rows converted — sync CONVERTED.md ↔ batch_progress.json."""
from __future__ import annotations

import argparse
import csv
import json
import re
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INVENTORY = ROOT / "docs" / "migration" / "c_inventory.csv"
PROGRESS = ROOT / "docs" / "migration" / "batch_progress.json"
CONVERTED = ROOT / "docs" / "migration" / "CONVERTED.md"

ROW_RE = re.compile(
    r"^\|\s*`([^`]+)`\s*\|\s*`([^`]*)`\s*\|\s*(.+?)\s*\|\s*$"
)


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def load_progress() -> dict:
    if not PROGRESS.exists():
        return {"updated": utc_now(), "waves": {}, "entries": []}
    return json.loads(PROGRESS.read_text(encoding="utf-8"))


def save_progress(data: dict) -> None:
    data["updated"] = utc_now()
    PROGRESS.parent.mkdir(parents=True, exist_ok=True)
    PROGRESS.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def recompute_wave_stats(entries: list[dict]) -> dict:
    waves: dict[str, dict[str, int]] = {}
    for e in entries:
        w = e["wave"]
        waves.setdefault(w, {"total": 0, "converted": 0, "pending": 0, "stubbed": 0})
        waves[w]["total"] += 1
        st = e.get("status", "pending")
        if st == "converted":
            waves[w]["converted"] += 1
        elif st == "stubbed":
            waves[w]["stubbed"] += 1
    for stats in waves.values():
        stats["pending"] = stats["total"] - stats["converted"]
    return waves


def inventory_row(path: str) -> dict[str, str] | None:
    if not INVENTORY.exists():
        return None
    norm = path.replace("\\", "/")
    with INVENTORY.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            if row["path"].replace("\\", "/") == norm:
                return row
    return None


def parse_converted_md(text: str) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line in text.splitlines():
        m = ROW_RE.match(line.strip())
        if not m:
            continue
        path, module, note = m.group(1), m.group(2), m.group(3).strip()
        if path.startswith("hbsd/") or path.startswith("kde/"):
            rows.append({"source": path.replace("\\", "/"), "module": module, "note": note})
    return rows


def append_converted_md(path: str, module: str, note: str) -> None:
    section_title = "## Wave 2 — Userland batch ports"
    line = f"| `{path}` | `{module}` | {note} |"
    header = (
        f"\n{section_title} ({datetime.now(timezone.utc).date().isoformat()})\n\n"
        "| hbsd source | PBSD module / stub | Notes |\n"
        "|-------------|-------------------|-------|\n"
    )
    text = CONVERTED.read_text(encoding="utf-8") if CONVERTED.exists() else (
        "# Converted C → PBSD C++23 Modules\n\n"
        "Tracking real ports and accepted migration stubs.\n"
    )
    if section_title in text:
        if f"| `{path}` |" in text:
            return
        insert_at = text.index(section_title)
        next_hdr = text.find("\n## ", insert_at + len(section_title))
        if next_hdr == -1:
            text = text.rstrip() + "\n" + line + "\n"
        else:
            text = text[:next_hdr].rstrip() + "\n" + line + "\n" + text[next_hdr:]
    else:
        text = text.rstrip() + header + line + "\n"
    CONVERTED.write_text(text, encoding="utf-8")


def mark_one(path: str, module: str, note: str) -> None:
    norm = path.replace("\\", "/")
    inv = inventory_row(norm)
    wave = inv["wave"] if inv else "wave2"
    loc = int(inv["loc_estimate"]) if inv and inv["loc_estimate"].isdigit() else 0

    progress = load_progress()
    entries = progress.setdefault("entries", [])
    found = False
    for e in entries:
        if e.get("source") == norm:
            e["status"] = "converted"
            e["converted_at"] = utc_now()
            if module:
                e["module"] = module
            if note:
                e["note"] = note
            found = True
            break
    if not found:
        entries.append(
            {
                "source": norm,
                "wave": wave,
                "loc_estimate": loc,
                "status": "converted",
                "module": module,
                "note": note,
                "converted_at": utc_now(),
            }
        )
    progress["entries"] = sorted(entries, key=lambda e: (e["wave"], e["source"]))
    progress["waves"] = recompute_wave_stats(progress["entries"])
    save_progress(progress)

    mod_display = module or "(hand port)"
    append_converted_md(norm, mod_display, note or "marked converted")


def sync_ledgers() -> tuple[int, int]:
    """Merge CONVERTED.md table rows into batch_progress; ensure MD lists JSON converted."""
    progress = load_progress()
    entries = progress.setdefault("entries", [])
    by_source = {e["source"]: e for e in entries}

    md_rows: list[dict[str, str]] = []
    if CONVERTED.exists():
        md_rows = parse_converted_md(CONVERTED.read_text(encoding="utf-8"))

    md_to_json = 0
    for row in md_rows:
        src = row["source"]
        inv = inventory_row(src)
        wave = inv["wave"] if inv else by_source.get(src, {}).get("wave", "wave2")
        loc = int(inv["loc_estimate"]) if inv and inv["loc_estimate"].isdigit() else 0
        prev = by_source.get(src, {})
        if prev.get("status") == "converted":
            continue
        by_source[src] = {
            **prev,
            "source": src,
            "wave": wave,
            "loc_estimate": prev.get("loc_estimate", loc),
            "status": "converted",
            "module": row["module"] or prev.get("module", ""),
            "note": row["note"] or prev.get("note", ""),
            "converted_at": prev.get("converted_at", utc_now()),
        }
        md_to_json += 1

    json_to_md = 0
    for e in entries:
        if e.get("status") != "converted":
            continue
        src = e["source"]
        mod = e.get("module", "") or "(hand port)"
        note = e.get("note", "batch_progress.json")
        before = CONVERTED.read_text(encoding="utf-8") if CONVERTED.exists() else ""
        append_converted_md(src, mod, note)
        after = CONVERTED.read_text(encoding="utf-8") if CONVERTED.exists() else ""
        if after != before:
            json_to_md += 1

    progress["entries"] = sorted(by_source.values(), key=lambda e: (e["wave"], e["source"]))
    progress["waves"] = recompute_wave_stats(progress["entries"])
    save_progress(progress)
    return md_to_json, json_to_md


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", nargs="?", help="Inventory path (posix) to mark converted")
    parser.add_argument("--module", default="", help="PBSD module name")
    parser.add_argument("--note", default="", help="Conversion note for CONVERTED.md")
    parser.add_argument(
        "--sync",
        action="store_true",
        help="Bidirectional sync CONVERTED.md ↔ batch_progress.json",
    )
    args = parser.parse_args()

    if args.sync:
        md_to_json, json_to_md = sync_ledgers()
        print(f"sync: md->json={md_to_json} json->md={json_to_md}")
        progress = load_progress()
        converted = sum(1 for e in progress.get("entries", []) if e.get("status") == "converted")
        print(f"batch_progress converted={converted} stubbed="
              f"{sum(1 for e in progress.get('entries', []) if e.get('status') == 'stubbed')}")
        return 0

    if not args.path:
        parser.error("path required unless --sync")
    mark_one(args.path, args.module, args.note)
    print(f"marked converted: {args.path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
