#!/usr/bin/env python3
"""Summarize docs/migration/c_inventory.csv by wave."""
from __future__ import annotations

import csv
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
p = ROOT / "docs" / "migration" / "c_inventory.csv"
rows = list(csv.DictReader(p.open(encoding="utf-8")))
print("rows", len(rows))
print("by_wave", Counter(r["wave"] for r in rows).most_common())
for wave in ("wave2", "wave3", "wave4", "wave5", "wave6", "wave7"):
    subset = [r for r in rows if r["wave"] == wave]
    subset.sort(key=lambda r: int(r["loc_estimate"]))
    print(f"\n{wave} smallest 12:")
    for r in subset[:12]:
        print(f"  {r['loc_estimate']:>6} {r['path']}")
