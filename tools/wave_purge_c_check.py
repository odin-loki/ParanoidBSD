#!/usr/bin/env python3
"""Wave 9 gate: inventory of residual owned .c files with firmware/asm exceptions."""
from __future__ import annotations

import argparse
import csv
import fnmatch
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INV = ROOT / "docs" / "migration" / "c_inventory.csv"
EXCEPTIONS = ROOT / "docs" / "migration" / "c_purge_exceptions.txt"
PROGRESS = ROOT / "docs" / "migration" / "batch_progress.json"
PBSD = ROOT / "pbsd"
PORTS = PBSD / "ports"
ACTIVE_WAVES = frozenset({"wave2", "wave3", "wave4", "wave5", "wave6", "wave7", "wave8"})


def load_exceptions(path: Path) -> list[str]:
    if not path.exists():
        return []
    patterns: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        patterns.append(line.replace("\\", "/"))
    return patterns


def is_excepted(rel_posix: str, patterns: list[str]) -> bool:
    for pat in patterns:
        if fnmatch.fnmatch(rel_posix, pat):
            return True
    return False


def load_inventory(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def count_hand_modules() -> int:
    if not PBSD.exists():
        return 0
    return sum(1 for m in PBSD.rglob("*.cppm") if "ports" not in m.parts)


def count_partition_modules() -> dict[str, int]:
    """Hand-authored nucleus modules under Waves 6–9 partition trees."""
    domains = ("net", "fs", "geom", "zfs", "stand", "arch", "bifrost", "compositor", "pkg")
    counts: dict[str, int] = {d: 0 for d in domains}
    if not PBSD.exists():
        return counts
    for mod in PBSD.rglob("*.cppm"):
        if "ports" in mod.parts:
            continue
        rel = mod.relative_to(PBSD)
        if not rel.parts:
            continue
        dom = rel.parts[0]
        if dom in counts and rel.name != f"pbsd.{dom}.cppm":
            counts[dom] += 1
    return counts


def count_wave_partition_modules() -> int:
    """Hand ports under net/fs/geom/zfs/stand/arch/bifrost/compositor/pkg."""
    roots = (
        "net", "fs", "geom", "zfs", "stand",
        "arch", "bifrost", "compositor", "pkg",
    )
    count = 0
    for root in roots:
        d = PBSD / root
        if not d.is_dir():
            continue
        for m in d.rglob("*.cppm"):
            if m.name.startswith("pbsd.") and m.name != f"pbsd.{root}.cppm":
                count += 1
    return count


def count_port_stubs() -> dict[str, int]:
    by_wave: dict[str, int] = {}
    if not PORTS.exists():
        return by_wave
    for stub in PORTS.rglob("*.cppm"):
        rel = stub.relative_to(PORTS)
        wave = rel.parts[0] if rel.parts else "?"
        by_wave[wave] = by_wave.get(wave, 0) + 1
    return by_wave


def load_batch_stats() -> dict[str, dict[str, int]]:
    if not PROGRESS.exists():
        return {}
    data = json.loads(PROGRESS.read_text(encoding="utf-8"))
    return data.get("waves", {})


def collect_metrics(rows: list[dict[str, str]], patterns: list[str]) -> dict:
    by_wave: dict[str, int] = {}
    owned_c = 0
    excepted = 0
    missing = 0
    active_rows: list[str] = []

    for r in rows:
        rel = r["path"].replace("\\", "/")
        wave = r.get("wave", "wave9")
        by_wave[wave] = by_wave.get(wave, 0) + 1

        if not rel.endswith(".c"):
            continue

        if is_excepted(rel, patterns):
            excepted += 1
            continue

        owned_c += 1
        if wave in ACTIVE_WAVES:
            active_rows.append(rel)

        if not (ROOT / rel).exists():
            missing += 1

    port_stubs = count_port_stubs()
    hand_modules = count_hand_modules()
    partition_modules = count_wave_partition_modules()
    batch = load_batch_stats()

    return {
        "inventory_rows": len(rows),
        "owned_c_post_exceptions": owned_c,
        "excepted_c": excepted,
        "stale_inventory_paths": missing,
        "active_wave_c_files": len(active_rows),
        "inventory_by_wave": dict(sorted(by_wave.items())),
        "hand_modules": hand_modules,
        "wave_partition_modules": partition_modules,
        "port_stubs_total": sum(port_stubs.values()),
        "port_stubs_by_wave": dict(sorted(port_stubs.items())),
        "batch_progress": batch,
        "gate_open": owned_c == 0 and len(active_rows) == 0,
    }


def print_human(metrics: dict) -> None:
    print(f"inventory entries: {metrics['inventory_rows']}")
    print(f"owned .c (post-exceptions): {metrics['owned_c_post_exceptions']}")
    print(f"excepted .c: {metrics['excepted_c']}")
    if metrics["stale_inventory_paths"]:
        print(
            f"stale inventory paths (missing on disk): {metrics['stale_inventory_paths']}",
            file=sys.stderr,
        )
    for w, n in metrics["inventory_by_wave"].items():
        print(f"  {w}: {n}")
    print(f"hand-authored .cppm: {metrics['hand_modules']}")
    print(f"wave 6–9 partition modules: {metrics.get('wave_partition_modules', 0)}")
    print(f"port stubs: {metrics['port_stubs_total']}")
    for wave, n in metrics["port_stubs_by_wave"].items():
        print(f"  port {wave}: {n}")
    batch = metrics.get("batch_progress") or {}
    if batch:
        print("batch_progress:")
        for wave, stats in sorted(batch.items()):
            print(
                f"  {wave}: total={stats.get('total', 0)} "
                f"converted={stats.get('converted', 0)} "
                f"stubbed={stats.get('stubbed', 0)}"
            )
    if metrics["active_wave_c_files"]:
        print(
            f"active conversion waves ({metrics['active_wave_c_files']} .c):",
            file=sys.stderr,
        )
    print(
        "PBSD 0.1 gate: owned .c must reach 0 (firmware/asm excepted). "
        "See docs/migration/PURGE_C.md."
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--gate",
        action="store_true",
        help="Exit 1 when owned .c remain (post-exceptions) or active waves non-empty",
    )
    parser.add_argument(
        "--metrics",
        action="store_true",
        help="Emit JSON metrics on stdout (human summary on stderr unless --quiet)",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="With --metrics, suppress human summary on stderr",
    )
    parser.add_argument(
        "--exceptions",
        type=Path,
        default=EXCEPTIONS,
        help="Path to exception glob list",
    )
    args = parser.parse_args()

    if not INV.exists():
        print("missing inventory — run tools/inventory_c_sources.py first", file=sys.stderr)
        return 2

    patterns = load_exceptions(args.exceptions)
    rows = load_inventory(INV)
    metrics = collect_metrics(rows, patterns)

    if args.metrics:
        print(json.dumps(metrics, indent=2, sort_keys=True))
        if not args.quiet:
            print("", file=sys.stderr)
            print(f"exception patterns: {len(patterns)} ({args.exceptions.relative_to(ROOT)})",
                  file=sys.stderr)
            print_human(metrics)
    else:
        print(f"exception patterns: {len(patterns)} ({args.exceptions.relative_to(ROOT)})")
        print_human(metrics)

    if args.gate and not metrics["gate_open"]:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
