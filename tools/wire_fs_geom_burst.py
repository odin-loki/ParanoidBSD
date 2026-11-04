#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Wire unionfs_vnops + kern_jail; create 16+ new geom modules with full CMake wiring."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

NEW_GEOM: list[tuple[str, str, str | None]] = [
    ("raid_tr_raid0", "hbsd/src/sys/geom/raid/tr_raid0.c", "pbsd_geom_raid"),
    ("raid_tr_raid1", "hbsd/src/sys/geom/raid/tr_raid1.c", "pbsd_geom_raid"),
    ("raid_tr_raid1e", "hbsd/src/sys/geom/raid/tr_raid1e.c", "pbsd_geom_raid"),
    ("raid_tr_concat", "hbsd/src/sys/geom/raid/tr_concat.c", "pbsd_geom_raid"),
    ("raid_md_intel", "hbsd/src/sys/geom/raid/md_intel.c", "pbsd_geom_raid"),
    ("raid_md_ddf", "hbsd/src/sys/geom/raid/md_ddf.c", "pbsd_geom_raid"),
    ("raid_md_nvidia", "hbsd/src/sys/geom/raid/md_nvidia.c", "pbsd_geom_raid"),
    ("raid_md_promise", "hbsd/src/sys/geom/raid/md_promise.c", "pbsd_geom_raid"),
    ("raid_md_sii", "hbsd/src/sys/geom/raid/md_sii.c", "pbsd_geom_raid"),
    ("raid_md_jmicron", "hbsd/src/sys/geom/raid/md_jmicron.c", "pbsd_geom_raid"),
    ("part_ldm", "hbsd/src/sys/geom/part/g_part_ldm.c", "pbsd_geom_part"),
    ("part_ebr", "hbsd/src/sys/geom/part/g_part_ebr.c", "pbsd_geom_part"),
    ("part_bsd64", "hbsd/src/sys/geom/part/g_part_bsd64.c", "pbsd_geom_part"),
    ("raid3_ctl", "hbsd/src/sys/geom/raid3/g_raid3_ctl.c", "pbsd_geom_raid3"),
    ("uzip_zstd", "hbsd/src/sys/geom/uzip/g_uzip_zstd.c", "pbsd_geom_uzip"),
    ("uzip_zlib", "hbsd/src/sys/geom/uzip/g_uzip_zlib.c", "pbsd_geom_uzip"),
    ("uzip_wrkthr", "hbsd/src/sys/geom/uzip/g_uzip_wrkthr.c", "pbsd_geom_uzip"),
    ("virstor_md", "hbsd/src/sys/geom/virstor/g_virstor_md.c", "pbsd_geom_virstor"),
]

WIRE_EXISTING: list[tuple[str, str, str, str | None, str]] = [
    # area, cmake_name, module_name, parent, rel_path
    ("fs", "unionfs_vnops", "unionfs_vnops", "pbsd_fs_unionfs", "fs/pbsd.fs.unionfs_vnops.cppm"),
    ("kernel", "kern_jail", "kern_jail", "pbsd_kernel_jail", "kernel/kern/pbsd.kernel.kern_jail.cppm"),
]


def cmake_target(area: str, name: str) -> str:
    return f"pbsd_{area}_{name.replace('.', '_')}"


def render_cppm(area: str, name: str, source: str) -> str:
    ns = f"pbsd::{area}::{name.replace('.', '_')}"
    title = name.replace("_", " ").title()
    return f"""module;
#include <cstdint>

export module pbsd.{area}.{name};

import pbsd.core;

/// PROVENANCE: {source} — {title} scaffold.
export namespace {ns} {{

enum class Op : unsigned char {{
    Init = 0,
    Validate = 1,
    Dispatch = 2,
}};

struct Ctx {{
    unsigned flags{{}};
    unsigned count{{}};
    bool active{{false}};
}};

[[nodiscard]] inline Status validate_op(Op op) noexcept {{
    switch (op) {{
    case Op::Init:
    case Op::Validate:
    case Op::Dispatch:
        return Status::Ok;
    default:
        return Status::Invalid;
    }}
}}

[[nodiscard]] inline Status init(Ctx& ctx) noexcept {{
    if (ctx.active) {{
        return Status::Busy;
    }}
    ctx.active = true;
    ctx.count = 0;
    return Status::Ok;
}}

[[nodiscard]] inline Status dispatch(Ctx& ctx, Op op) noexcept {{
    if (validate_op(op) != Status::Ok) {{
        return Status::Invalid;
    }}
    if (!ctx.active) {{
        return Status::Invalid;
    }}
    ++ctx.count;
    return Status::Ok;
}}

}} // namespace {ns}
"""


def render_c(source: str) -> str:
    return f"/* Reference logic from {source} (dual-world). */\n\n/* stub */\n"


def cmake_block(area: str, name: str, rel: str, parent: str | None) -> str:
    tgt = cmake_target(area, name)
    deps = "pbsd_core"
    if parent:
        deps = f"pbsd_core {parent}"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {rel})
target_link_libraries({tgt} PUBLIC {deps})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def has_word(text: str, word: str) -> bool:
    return re.search(rf"\b{re.escape(word)}\b", text) is not None


def patch_cmake_targets(blocks: list[str], anchor: str) -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    added = 0
    for block in blocks:
        tgt = re.search(r"if\(NOT TARGET (pbsd_\w+)\)", block)
        if not tgt:
            continue
        name = tgt.group(1)
        if has_word(cmake, name):
            continue
        if anchor in cmake:
            cmake = cmake.replace(anchor, block + "\n" + anchor, 1)
            added += 1
    if added:
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate_links(area: str, libs: list[str]) -> int:
    if area == "kernel":
        pattern = r"(target_link_libraries\(pbsd_kernel INTERFACE[^\)]*)"
    else:
        pattern = rf"(target_link_libraries\(pbsd_{area} PUBLIC [^\)]*)"
    cmake = CMAKE.read_text(encoding="utf-8")
    m = re.search(pattern, cmake, re.DOTALL)
    if not m:
        return 0
    link_block = m.group(1)
    added = 0
    for lib in libs:
        if not has_word(link_block, lib):
            link_block = link_block.rstrip() + f" {lib}"
            added += 1
    if added:
        cmake = cmake[: m.start(1)] + link_block + ")" + cmake[m.end(0) :]
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate_exports(area: str, names: list[str]) -> int:
    agg = PBSD / area / f"pbsd.{area}.cppm"
    if not agg.exists():
        return 0
    text = agg.read_text(encoding="utf-8")
    added = 0
    for name in names:
        imp = f"export import pbsd.{area}.{name};"
        if imp in text:
            continue
        for mk in ("/// Wave 6", "export namespace"):
            idx = text.find(mk)
            if idx != -1:
                text = text[:idx] + imp + "\n" + text[idx:]
                added += 1
                break
    if added:
        agg.write_text(text, encoding="utf-8", newline="\n")
    return added


def main() -> None:
    wired: list[str] = []

    # --- new geom modules ---
    geom_blocks: list[str] = []
    geom_libs: list[str] = []
    geom_exports: list[str] = []
    for name, source, parent in NEW_GEOM:
        path = PBSD / "geom" / f"pbsd.geom.{name}.cppm"
        cpath = PBSD / "geom" / f"pbsd.geom.{name}.c"
        if not path.exists():
            path.write_text(render_cppm("geom", name, source), encoding="utf-8", newline="\n")
        if not cpath.exists():
            cpath.write_text(render_c(source), encoding="utf-8", newline="\n")
        rel = f"geom/pbsd.geom.{name}.cppm"
        geom_blocks.append(cmake_block("geom", name, rel, parent))
        geom_libs.append(cmake_target("geom", name))
        geom_exports.append(name)
        wired.append(str(path.relative_to(ROOT)).replace("\\", "/"))

    patch_cmake_targets(geom_blocks, "if(NOT TARGET pbsd_geom)\nadd_library(pbsd_geom)")
    patch_aggregate_exports("geom", geom_exports)
    patch_aggregate_links("geom", geom_libs)

    # --- wire existing fs + kernel ---
    fs_blocks: list[str] = []
    fs_libs: list[str] = []
    fs_exports: list[str] = []
    kernel_blocks: list[str] = []
    kernel_libs: list[str] = []

    for area, cmake_name, mod_name, parent, rel in WIRE_EXISTING:
        if not has_word(CMAKE.read_text(encoding="utf-8"), cmake_target(area, cmake_name)):
            block = cmake_block(area, cmake_name, rel, parent)
            if area == "fs":
                fs_blocks.append(block)
            else:
                kernel_blocks.append(block)
        lib = cmake_target(area, cmake_name)
        if area == "fs":
            fs_libs.append(lib)
            fs_exports.append(mod_name)
        else:
            kernel_libs.append(lib)
        wired.append(rel.replace("\\", "/"))
        if area == "kernel":
            cpath = PBSD / "kernel" / "kern" / "pbsd.kernel.kern_jail.c"
            if not cpath.exists():
                cpath.write_text(render_c("hbsd/src/sys/kern/kern_jail.c"), encoding="utf-8", newline="\n")

    if fs_blocks:
        patch_cmake_targets(fs_blocks, "if(NOT TARGET pbsd_fs)\nadd_library(pbsd_fs)")
    if kernel_blocks:
        patch_cmake_targets(kernel_blocks, "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)")
    patch_aggregate_exports("fs", fs_exports)
    patch_aggregate_links("fs", fs_libs)
    patch_aggregate_links("kernel", kernel_libs)

    print(f"Wired {len(wired)} paths:")
    for p in wired:
        print(f"  {p}")


if __name__ == "__main__":
    main()
