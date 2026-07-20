#!/usr/bin/env python3
"""Append batch3 modules to pbsd/kde/CMakeLists.txt without duplicates."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CMAKE = ROOT / "pbsd" / "kde" / "CMakeLists.txt"
MANIFEST = ROOT / "tools" / "kde_wave3_batch3_modules.txt"

LOGGING_SUFFIX = ".logging.cppm"
EFFECTS_PREFIX = "kwin/effects/"
PLASMA_PREFIXES = ("plasma.",)
FRAMEWORKS_PREFIX = "frameworks/"


def classify(rel: str) -> str:
    if rel.endswith(LOGGING_SUFFIX) or "/logging" in rel:
        return "logging"
    if rel.startswith(EFFECTS_PREFIX):
        return "effects"
    if rel.startswith(PLASMA_PREFIXES):
        return "plasma"
    if rel.startswith(FRAMEWORKS_PREFIX):
        return "frameworks"
    if rel.startswith("kwin/") or rel.startswith("kwin."):
        return "kwin"
    return "kwin"


def extract_list(content: str, var: str) -> list[str]:
    m = re.search(rf"set\({var}\s*\n(.*?)\)", content, re.S)
    if not m:
        raise SystemExit(f"missing {var}")
    return re.findall(r"^\s+([\w./-]+\.cppm)\s*$", m.group(1), re.M)


def replace_list(content: str, var: str, items: list[str]) -> str:
    block = "set(" + var + "\n" + "".join(f"    {i}\n" for i in items) + ")"
    return re.sub(rf"set\({var}\s*\n.*?\)", block, content, count=1, flags=re.S)


def main() -> int:
    new_modules = [ln.strip() for ln in MANIFEST.read_text(encoding="utf-8").splitlines() if ln.strip()]
    content = CMAKE.read_text(encoding="utf-8")

    lists = {
        "logging": extract_list(content, "PBSD_KDE_KWIN_LOGGING_MODULES"),
        "effects": extract_list(content, "PBSD_KDE_KWIN_EFFECT_MODULES"),
        "frameworks": extract_list(content, "PBSD_KDE_FRAMEWORKS_MODULES"),
        "plasma": extract_list(content, "PBSD_KDE_PLASMA_MODULES"),
        "kwin": extract_list(content, "PBSD_KDE_KWIN_MODULES"),
    }

    added = 0
    for rel in new_modules:
        bucket = classify(rel)
        if rel not in lists[bucket]:
            lists[bucket].append(rel)
            added += 1

    content = replace_list(content, "PBSD_KDE_KWIN_LOGGING_MODULES", lists["logging"])
    content = replace_list(content, "PBSD_KDE_KWIN_EFFECT_MODULES", lists["effects"])
    content = replace_list(content, "PBSD_KDE_FRAMEWORKS_MODULES", lists["frameworks"])
    content = replace_list(content, "PBSD_KDE_PLASMA_MODULES", lists["plasma"])
    content = replace_list(content, "PBSD_KDE_KWIN_MODULES", lists["kwin"])

    CMAKE.write_text(content, encoding="utf-8")
    total = sum(len(v) for v in lists.values())
    print(f"added {added} modules; cmake lists total entries: {total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
