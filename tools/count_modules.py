#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Count PBSD C++23 .cppm modules and port stubs."""
from __future__ import annotations

import json
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
PORTS = PBSD / "ports"
PROGRESS = ROOT / "docs" / "migration" / "batch_progress.json"


def main() -> int:
    mods = list(PBSD.rglob("*.cppm"))
    port_stubs = list(PORTS.rglob("*.cppm")) if PORTS.exists() else []
    hand = [m for m in mods if "ports" not in m.parts]

    print(f"hand_modules={len(hand)}")
    print(f"port_stubs={len(port_stubs)}")
    print(f"total_cppm={len(mods)}")

    by_top: Counter[str] = Counter()
    for m in hand:
        by_top[m.relative_to(PBSD).parts[0]] += 1
    for k in sorted(by_top):
        print(f"  {k}: {by_top[k]}")

    if PORTS.exists():
        by_wave: Counter[str] = Counter()
        for m in port_stubs:
            rel = m.relative_to(PORTS)
            by_wave[rel.parts[0] if rel.parts else "?"] += 1
        print("port_stubs_by_wave:")
        for wave in sorted(by_wave):
            print(f"  {wave}: {by_wave[wave]}")

    if PROGRESS.exists():
        data = json.loads(PROGRESS.read_text(encoding="utf-8"))
        waves = data.get("waves", {})
        if waves:
            print("batch_progress:")
            for wave, stats in sorted(waves.items()):
                print(
                    f"  {wave}: total={stats.get('total', 0)} "
                    f"converted={stats.get('converted', 0)} "
                    f"stubbed={stats.get('stubbed', 0)}"
                )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
