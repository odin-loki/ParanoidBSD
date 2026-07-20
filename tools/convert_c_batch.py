#!/usr/bin/env python3
"""Generate pbsd/ports/<wave>/ C++23 module stubs from c_inventory.csv."""
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
PORTS = ROOT / "pbsd" / "ports"

HEADER_SUFFIXES = {".h", ".hpp", ".hh"}


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def load_inventory(path: Path = INVENTORY) -> list[dict[str, str]]:
    with path.open(encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


def load_progress(path: Path = PROGRESS) -> dict:
    if not path.exists():
        return {"updated": utc_now(), "waves": {}, "entries": []}
    return json.loads(path.read_text(encoding="utf-8"))


def save_progress(data: dict, path: Path = PROGRESS) -> None:
    data["updated"] = utc_now()
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=False) + "\n", encoding="utf-8")


def sanitize_module_segment(name: str) -> str:
    seg = re.sub(r"[^a-zA-Z0-9_]", "_", name)
    if not seg or seg[0].isdigit():
        seg = f"_{seg}"
    return seg.lower()


def module_name_for(source_posix: str, wave: str) -> str:
    rel = Path(source_posix)
    stem = sanitize_module_segment(rel.stem)
    parts = [sanitize_module_segment(p) for p in rel.parts]
    wave_seg = sanitize_module_segment(wave)
    return "pbsd.port." + ".".join([wave_seg, *parts[:-1], stem])


def stub_relpath(source_posix: str, wave: str) -> Path:
    src = Path(source_posix)
    return Path(wave) / src.with_suffix(".cppm")


def default_imports(source_posix: str) -> list[str]:
    p = source_posix.replace("\\", "/")
    imports: list[str] = ["import pbsd.core;"]
    if p.startswith("hbsd/src/lib/libcapsicum") or "capsicum" in p:
        imports.insert(0, "export import pbsd.userland.capsicum.helpers;")
        imports.insert(0, "export import pbsd.userland.capsicum;")
    elif p.startswith("hbsd/src/bin/") or p.startswith("hbsd/src/sbin/"):
        imports.insert(0, "export import pbsd.userland.capsicum.helpers;")
    return imports


def extern_c_block(source_posix: str, suffix: str) -> str:
    p = source_posix.replace("\\", "/")
    if suffix in HEADER_SUFFIXES:
        return (
            f"// Header bridge — replace #include of {p}\n"
            "// with imports from pbsd.userland.capsicum.helpers during hand-port.\n"
        )
    stem = Path(p).stem
    if stem == "main" or p.startswith("hbsd/src/bin/") or p.startswith("hbsd/src/sbin/"):
        return (
            'extern "C" {\n'
            f"// Legacy TU entry — hand-port algorithm from {p}\n"
            "int main(int argc, char* argv[]);\n"
            "}\n"
        )
    return (
        'extern "C" {\n'
        f"// Legacy TU symbols — hand-port from {p}\n"
        f"// void {stem}_init(void);\n"
        "}\n"
    )


def render_stub(source_posix: str, wave: str, loc: str, status: str) -> str:
    mod = module_name_for(source_posix, wave)
    suffix = Path(source_posix).suffix.lower()
    imports = "\n".join(default_imports(source_posix))
    ns_parts = [sanitize_module_segment(p) for p in Path(source_posix).parts]
    ns = "::".join(["pbsd", "port", sanitize_module_segment(wave), *ns_parts[:-1], sanitize_module_segment(Path(source_posix).stem)])
    extern = extern_c_block(source_posix, suffix)
    return f"""export module {mod};

module;
{extern}
{imports}

/// Auto-generated migration stub — status: {status}
/// @pbsd-migration-meta source={source_posix} wave={wave} loc={loc}
export namespace {ns} {{

[[nodiscard]] inline Status migration_status() noexcept {{
    return Status::NotImplemented;
}}

}} // namespace {ns}
"""


def render_meta(source_posix: str, wave: str, loc: str, status: str, stub_path: Path) -> dict:
    return {
        "source": source_posix,
        "wave": wave,
        "loc_estimate": int(loc) if loc.isdigit() else 0,
        "status": status,
        "stub": stub_path.as_posix(),
        "module": module_name_for(source_posix, wave),
        "generated": utc_now(),
    }


def discover_header_sources(prefix: str) -> list[dict[str, str]]:
    """Include header-only trees (e.g. libcapsicum) not present in c_inventory.csv."""
    base = ROOT / prefix.replace("/", "\\")
    if not base.is_dir():
        return []
    rows: list[dict[str, str]] = []
    for path in sorted(base.rglob("*")):
        if path.suffix.lower() not in HEADER_SUFFIXES:
            continue
        if ".git" in path.parts:
            continue
        rel = path.relative_to(ROOT).as_posix()
        try:
            loc = sum(1 for _ in path.open("rb"))
        except OSError:
            loc = 0
        rows.append({"path": rel, "wave": "wave2", "loc_estimate": str(loc)})
    return rows


def matches_prefix(path: str, prefixes: list[str]) -> bool:
    p = path.replace("\\", "/")
    return any(p.startswith(pref if pref.endswith("/") else pref + "/") or p == pref for pref in prefixes)


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
        else:
            waves[w]["pending"] += 1
    for stats in waves.values():
        stats["pending"] = stats["total"] - stats["converted"]
    return waves


def merge_entries(existing: list[dict], new_rows: list[dict]) -> list[dict]:
    by_path = {e["source"]: e for e in existing}
    for row in new_rows:
        src = row["source"]
        prev = by_path.get(src, {})
        by_path[src] = {**prev, **row}
    return sorted(by_path.values(), key=lambda e: (e["wave"], e["source"]))


def generate_batch(
    wave: str,
    prefixes: list[str],
    *,
    include_headers: bool = False,
    limit: int = 0,
    skip_stubbed: bool = False,
    dry_run: bool = False,
) -> tuple[int, dict]:
    if not INVENTORY.exists():
        print("missing inventory — run tools/inventory_c_sources.py first", file=sys.stderr)
        return 0, {}

    inv = load_inventory()
    selected = [r for r in inv if r["wave"] == wave and matches_prefix(r["path"], prefixes)]

    if include_headers:
        for pref in prefixes:
            for row in discover_header_sources(pref):
                if row["path"] not in {r["path"] for r in selected}:
                    selected.append(row)

    selected.sort(key=lambda r: (int(r.get("loc_estimate") or 0), r["path"]))
    if limit > 0:
        selected = selected[:limit]

    progress = load_progress()
    stubbed_sources = {
        e["source"]
        for e in progress.get("entries", [])
        if e.get("status") in {"stubbed", "converted"}
    }
    new_entries: list[dict] = []
    written = 0

    for row in selected:
        src = row["path"].replace("\\", "/")
        if skip_stubbed and src in stubbed_sources:
            continue
        loc = row.get("loc_estimate", "0")
        prev = next((e for e in progress.get("entries", []) if e.get("source") == src), None)
        status = prev.get("status", "pending") if prev else "pending"
        if status == "converted":
            stub_status = "converted"
        else:
            stub_status = "stubbed"
            status = "stubbed"

        rel_stub = stub_relpath(src, wave)
        stub_path = PORTS / rel_stub
        meta_path = stub_path.with_suffix(".cppm.meta.json")

        if not dry_run:
            stub_path.parent.mkdir(parents=True, exist_ok=True)
            stub_path.write_text(render_stub(src, wave, loc, stub_status), encoding="utf-8")
            meta = render_meta(src, wave, loc, stub_status, Path("pbsd/ports") / rel_stub)
            meta_path.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")

        new_entries.append(
            {
                "source": src,
                "wave": wave,
                "loc_estimate": int(loc) if str(loc).isdigit() else 0,
                "status": status,
                "stub": (Path("pbsd/ports") / rel_stub).as_posix(),
                "module": module_name_for(src, wave),
            }
        )
        written += 1

    progress["entries"] = merge_entries(progress.get("entries", []), new_entries)
    progress["waves"] = recompute_wave_stats(progress["entries"])
    progress["last_batch"] = {
        "wave": wave,
        "prefixes": prefixes,
        "generated": written,
        "at": utc_now(),
    }

    if not dry_run:
        save_progress(progress)

    return written, progress


def print_report(progress: dict) -> None:
    print(f"batch_progress updated: {progress.get('updated', '?')}")
    for wave, stats in sorted(progress.get("waves", {}).items()):
        print(
            f"  {wave}: total={stats.get('total', 0)} "
            f"converted={stats.get('converted', 0)} "
            f"stubbed={stats.get('stubbed', 0)} "
            f"pending={stats.get('pending', 0)}"
        )
    last = progress.get("last_batch")
    if last:
        print(
            f"last_batch: {last.get('wave')} prefixes={last.get('prefixes')} "
            f"generated={last.get('generated')}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wave", default="wave2", help="Inventory wave tag (default: wave2)")
    parser.add_argument(
        "--prefix",
        action="append",
        dest="prefixes",
        help="Path prefix under repo root (repeatable)",
    )
    parser.add_argument(
        "--include-headers",
        action="store_true",
        help="Also emit stubs for .h files under prefixes (libcapsicum)",
    )
    parser.add_argument("--limit", type=int, default=0, help="Max rows to generate (0 = all)")
    parser.add_argument(
        "--skip-stubbed",
        action="store_true",
        help="Skip inventory rows already stubbed/converted in batch_progress.json",
    )
    parser.add_argument("--dry-run", action="store_true", help="Compute progress only")
    parser.add_argument(
        "--report",
        action="store_true",
        help="Print batch_progress.json summary and exit",
    )
    args = parser.parse_args()

    if args.report:
        progress = load_progress()
        if not progress.get("entries"):
            print("batch_progress.json empty — run a batch first")
            return 0
        print_report(progress)
        return 0

    prefixes = args.prefixes or ["hbsd/src/lib/libcapsicum", "hbsd/src/bin/echo"]
    written, progress = generate_batch(
        args.wave,
        prefixes,
        include_headers=args.include_headers or any("libcapsicum" in p for p in prefixes),
        limit=args.limit,
        skip_stubbed=args.skip_stubbed,
        dry_run=args.dry_run,
    )
    print(f"generated {written} stub(s) under pbsd/ports/{args.wave}/")
    print_report(progress)
    return 0


if __name__ == "__main__":
    sys.exit(main())
