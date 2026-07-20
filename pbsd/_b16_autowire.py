#!/usr/bin/env python3
"""Post-rsync: ensure aggregate export imports have CMake targets + link edges."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else ".")

SPECS = [
    # aggregate, prefix, dir, cmake_path, tgt_prefix, insert_before, aggregate_target
    ("net/pbsd.net.cppm", "pbsd.net", "net", "CMakeLists.txt", "pbsd_net_", "if(NOT TARGET pbsd_net)", "pbsd_net"),
    ("zfs/pbsd.zfs.cppm", "pbsd.zfs", "zfs", "CMakeLists.txt", "pbsd_zfs_", "if(NOT TARGET pbsd_zfs)", "pbsd_zfs"),
    ("fs/pbsd.fs.cppm", "pbsd.fs", "fs", "CMakeLists.txt", "pbsd_fs_", "if(NOT TARGET pbsd_fs)", "pbsd_fs"),
    ("geom/pbsd.geom.cppm", "pbsd.geom", "geom", "CMakeLists.txt", "pbsd_geom_", "if(NOT TARGET pbsd_geom)", "pbsd_geom"),
    ("bifrost/pbsd.bifrost.cppm", "pbsd.bifrost", "bifrost", "bifrost/CMakeLists.txt", "pbsd_bifrost_", "add_library(pbsd_bifrost)", "pbsd_bifrost"),
]


def wb_in(text: str, token: str) -> bool:
    return re.search(rf"(?<![A-Za-z0-9_]){re.escape(token)}(?![A-Za-z0-9_])", text) is not None


def link_body(cmake: str, agg_tgt: str) -> str | None:
    m = re.search(
        rf"target_link_libraries\(\s*{re.escape(agg_tgt)}\s+PUBLIC(.*?)\)\s*\n",
        cmake,
        re.S,
    )
    return m.group(1) if m else None


def ensure(spec: tuple[str, str, str, str, str, str, str]) -> int:
    agg_rel, prefix, dir_rel, cmake_rel, tgt_prefix, before, agg_tgt = spec
    agg_path = ROOT / agg_rel
    cmake_path = ROOT / cmake_rel
    if not agg_path.exists() or not cmake_path.exists():
        return 0

    agg = agg_path.read_text(encoding="utf-8", errors="replace")
    cmake = cmake_path.read_text(encoding="utf-8", errors="replace")
    mods = re.findall(rf"export import {re.escape(prefix)}\.([A-Za-z0-9_]+);", agg)
    added = 0
    blocks: list[str] = []
    link_adds: list[str] = []
    body = link_body(cmake, agg_tgt) or ""

    for m in mods:
        tgt = f"{tgt_prefix}{m}"
        if dir_rel == "bifrost":
            src = f"{prefix}.{m}.cppm"
            src_path = ROOT / "bifrost" / src
        else:
            src = f"{dir_rel}/{prefix}.{m}.cppm"
            src_path = ROOT / src
        if not src_path.exists():
            new_agg = re.sub(
                rf"^export import {re.escape(prefix)}\.{re.escape(m)};\n?",
                "",
                agg,
                count=1,
                flags=re.M,
            )
            if new_agg != agg:
                agg = new_agg
                print(f"DROP export {prefix}.{m} (missing file)")
            continue

        has_target = f"add_library({tgt})" in cmake or f"TARGET {tgt}" in cmake
        linked = wb_in(body, tgt)
        if has_target:
            if not linked:
                link_adds.append(tgt)
                print(f"LINK missing {tgt}")
            continue

        blocks.append(
            f"\nif(NOT TARGET {tgt})\n"
            f"add_library({tgt})\n"
            f"target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {src})\n"
            f"target_link_libraries({tgt} PUBLIC pbsd_core)\n"
            f"target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})\n"
            f"endif()\n"
        )
        link_adds.append(tgt)
        added += 1
        print(f"ADD target {tgt}")

    if blocks:
        if before not in cmake:
            print(f"WARN: insert point missing for {prefix}: {before}")
        else:
            cmake = cmake.replace(before, "".join(blocks) + "\n" + before, 1)

    if link_adds:
        def inject(match: re.Match[str]) -> str:
            text = match.group(0)
            missing = [t for t in link_adds if not wb_in(text, t)]
            if not missing:
                return text
            return text[:-1] + " " + " ".join(missing) + ")"

        cmake2, n = re.subn(
            rf"target_link_libraries\(\s*{re.escape(agg_tgt)}\s+PUBLIC.*?\)\s*\n",
            inject,
            cmake,
            count=1,
            flags=re.S,
        )
        if n:
            cmake = cmake2
            print(f"LINK injected {len(link_adds)} into {agg_tgt}")
        else:
            print(f"WARN: could not inject links for {agg_tgt}")

    cmake_path.write_text(cmake, encoding="utf-8")
    agg_path.write_text(agg, encoding="utf-8")
    return added + len(link_adds)


def main() -> int:
    total = 0
    for spec in SPECS:
        total += ensure(spec)
    print(f"AUTOWIRE_ADDED={total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
