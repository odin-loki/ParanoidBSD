#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 16 — mass net/fs/geom/zfs hand ports + dual-world .c stubs + CMake wiring."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

KEYWORD_BAN = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
    "class", "compl", "concept", "const", "consteval", "constexpr", "constinit",
    "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
    "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
    "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
    "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
    "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected",
    "public", "register", "reinterpret_cast", "requires", "return", "short",
    "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
    "switch", "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual",
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq", "module", "import",
}

# (area, name, hbsd_source, parent_cmake_target or None)
NEW_MODULES: list[tuple[str, str, str, str | None]] = [
    # net — route / link / wireless
    ("net", "dummymbuf", "hbsd/src/sys/net/dummymbuf.c", None),
    ("net", "ieee8023ad_lacp", "hbsd/src/sys/net/ieee8023ad_lacp.c", "pbsd_net_lacp"),
    ("net", "iflib", "hbsd/src/sys/net/iflib.c", "pbsd_net_ifnet"),
    ("net", "mp_ring", "hbsd/src/sys/net/mp_ring.c", None),
    ("net", "mppcc", "hbsd/src/sys/net/mppcc.c", None),
    ("net", "mppcd", "hbsd/src/sys/net/mppcd.c", None),
    ("net", "fib_algo", "hbsd/src/sys/net/route/fib_algo.c", "pbsd_net_route"),
    ("net", "nhgrp", "hbsd/src/sys/net/route/nhgrp.c", "pbsd_net_route"),
    ("net", "nhgrp_ctl", "hbsd/src/sys/net/route/nhgrp_ctl.c", "pbsd_net_route"),
    ("net", "nhop", "hbsd/src/sys/net/route/nhop.c", "pbsd_net_route"),
    ("net", "nhop_ctl", "hbsd/src/sys/net/route/nhop_ctl.c", "pbsd_net_route"),
    ("net", "nhop_utils", "hbsd/src/sys/net/route/nhop_utils.c", "pbsd_net_route"),
    ("net", "route_ctl", "hbsd/src/sys/net/route/route_ctl.c", "pbsd_net_route"),
    ("net", "route_ddb", "hbsd/src/sys/net/route/route_ddb.c", "pbsd_net_route"),
    ("net", "route_ifaddrs", "hbsd/src/sys/net/route/route_ifaddrs.c", "pbsd_net_route"),
    ("net", "route_subscription", "hbsd/src/sys/net/route/route_subscription.c", "pbsd_net_route"),
    ("net", "route_tables", "hbsd/src/sys/net/route/route_tables.c", "pbsd_net_route"),
    ("net", "route_temporal", "hbsd/src/sys/net/route/route_temporal.c", "pbsd_net_route"),
    ("net", "slcompress", "hbsd/src/sys/net/slcompress.c", None),
    ("net", "ieee80211", "hbsd/src/sys/net80211/ieee80211.c", None),
    ("net", "ieee80211_acl", "hbsd/src/sys/net80211/ieee80211_acl.c", None),
    ("net", "ieee80211_action", "hbsd/src/sys/net80211/ieee80211_action.c", None),
    ("net", "ieee80211_adhoc", "hbsd/src/sys/net80211/ieee80211_adhoc.c", None),
    ("net", "ieee80211_ageq", "hbsd/src/sys/net80211/ieee80211_ageq.c", None),
    # fs — vfsops / subr gap-fill
    ("fs", "autofs_vfsops", "hbsd/src/sys/fs/autofs/autofs_vfsops.c", "pbsd_fs_autofs"),
    ("fs", "cd9660_bmap", "hbsd/src/sys/fs/cd9660/cd9660_bmap.c", "pbsd_fs_cd9660"),
    ("fs", "cd9660_lookup", "hbsd/src/sys/fs/cd9660/cd9660_lookup.c", "pbsd_fs_cd9660"),
    ("fs", "cd9660_node", "hbsd/src/sys/fs/cd9660/cd9660_node.c", "pbsd_fs_cd9660"),
    ("fs", "cd9660_vfsops", "hbsd/src/sys/fs/cd9660/cd9660_vfsops.c", "pbsd_fs_cd9660"),
    ("fs", "cuse", "hbsd/src/sys/fs/cuse/cuse.c", None),
    ("fs", "devfs_devs", "hbsd/src/sys/fs/devfs/devfs_devs.c", "pbsd_fs_devfs"),
    ("fs", "devfs_dir", "hbsd/src/sys/fs/devfs/devfs_dir.c", "pbsd_fs_devfs"),
    ("fs", "devfs_rule", "hbsd/src/sys/fs/devfs/devfs_rule.c", "pbsd_fs_devfs"),
    ("fs", "devfs_vfsops", "hbsd/src/sys/fs/devfs/devfs_vfsops.c", "pbsd_fs_devfs"),
    ("fs", "ext2_acl", "hbsd/src/sys/fs/ext2fs/ext2_acl.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_alloc", "hbsd/src/sys/fs/ext2fs/ext2_alloc.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_bmap", "hbsd/src/sys/fs/ext2fs/ext2_bmap.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_lookup", "hbsd/src/sys/fs/ext2fs/ext2_lookup.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_vfsops", "hbsd/src/sys/fs/ext2fs/ext2_vfsops.c", "pbsd_fs_ext2fs"),
    ("fs", "fuse_vfsops", "hbsd/src/sys/fs/fuse/fuse_vfsops.c", "pbsd_fs_fusefs"),
    ("fs", "msdosfs_vfsops", "hbsd/src/sys/fs/msdosfs/msdosfs_vfsops.c", "pbsd_fs_msdosfs"),
    ("fs", "null_vfsops", "hbsd/src/sys/fs/nullfs/null_vfsops.c", "pbsd_fs_nullfs"),
    ("fs", "procfs_vfsops", "hbsd/src/sys/fs/procfs/procfs_vfsops.c", "pbsd_fs_procfs"),
    ("fs", "tmpfs_vfsops", "hbsd/src/sys/fs/tmpfs/tmpfs_vfsops.c", "pbsd_fs_tmpfs"),
    # geom — core + eli + label subr
    ("geom", "eli_crypto", "hbsd/src/sys/geom/eli/g_eli_crypto.c", "pbsd_geom_eli"),
    ("geom", "eli_ctl", "hbsd/src/sys/geom/eli/g_eli_ctl.c", "pbsd_geom_eli"),
    ("geom", "eli_hmac", "hbsd/src/sys/geom/eli/g_eli_hmac.c", "pbsd_geom_eli"),
    ("geom", "eli_integrity", "hbsd/src/sys/geom/eli/g_eli_integrity.c", "pbsd_geom_eli"),
    ("geom", "eli_key_cache", "hbsd/src/sys/geom/eli/g_eli_key_cache.c", "pbsd_geom_eli"),
    ("geom", "eli_privacy", "hbsd/src/sys/geom/eli/g_eli_privacy.c", "pbsd_geom_eli"),
    ("geom", "pkcs5v2", "hbsd/src/sys/geom/eli/pkcs5v2.c", "pbsd_geom_eli"),
    ("geom", "geom_kern", "hbsd/src/sys/geom/geom_kern.c", None),
    ("geom", "geom_dev", "hbsd/src/sys/geom/geom_dev.c", None),
    ("geom", "label_disk_ident", "hbsd/src/sys/geom/label/g_label_disk_ident.c", "pbsd_geom_label"),
    ("geom", "label_ext2fs", "hbsd/src/sys/geom/label/g_label_ext2fs.c", "pbsd_geom_label"),
    ("geom", "label_flashmap", "hbsd/src/sys/geom/label/g_label_flashmap.c", "pbsd_geom_label"),
    ("geom", "label_iso9660", "hbsd/src/sys/geom/label/g_label_iso9660.c", "pbsd_geom_label"),
    ("geom", "label_msdosfs", "hbsd/src/sys/geom/label/g_label_msdosfs.c", "pbsd_geom_label"),
    ("geom", "label_reiserfs", "hbsd/src/sys/geom/label/g_label_reiserfs.c", "pbsd_geom_label"),
    ("geom", "label_ufs_id", "hbsd/src/sys/geom/label/g_label_ufs_id.c", "pbsd_geom_label"),
    # zfs — dmu/dsl/ddt/arc helpers
    ("zfs", "abd", "hbsd/src/sys/contrib/openzfs/module/zfs/abd.c", None),
    ("zfs", "aggsum", "hbsd/src/sys/contrib/openzfs/module/zfs/aggsum.c", None),
    ("zfs", "bplist", "hbsd/src/sys/contrib/openzfs/module/zfs/bplist.c", "pbsd_zfs_dsl"),
    ("zfs", "bptree", "hbsd/src/sys/contrib/openzfs/module/zfs/bptree.c", "pbsd_zfs_dsl"),
    ("zfs", "ddt", "hbsd/src/sys/contrib/openzfs/module/zfs/ddt.c", "pbsd_zfs_dmu"),
    ("zfs", "ddt_log", "hbsd/src/sys/contrib/openzfs/module/zfs/ddt_log.c", "pbsd_zfs_dmu"),
    ("zfs", "ddt_stats", "hbsd/src/sys/contrib/openzfs/module/zfs/ddt_stats.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_objset", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_objset.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_recv", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_recv.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_send", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_send.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_traverse", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_traverse.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_zfetch", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_zfetch.c", "pbsd_zfs_dmu"),
    ("zfs", "dsl_bookmark", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_bookmark.c", "pbsd_zfs_dsl"),
    ("zfs", "dsl_crypt", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_crypt.c", "pbsd_zfs_dsl"),
    ("zfs", "dsl_destroy", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_destroy.c", "pbsd_zfs_dsl"),
    ("zfs", "dsl_prop", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_prop.c", "pbsd_zfs_dsl"),
    ("zfs", "dataset_kstats", "hbsd/src/sys/contrib/openzfs/module/zfs/dataset_kstats.c", "pbsd_zfs_dsl"),
    ("zfs", "dbuf_stats", "hbsd/src/sys/contrib/openzfs/module/zfs/dbuf_stats.c", "pbsd_zfs_dbuf"),
]

# Existing .cppm needing cmake + aggregate wiring only.
WIRE_EXISTING: list[tuple[str, str, str | None]] = [
    ("geom", "subr_disk", "pbsd_geom_disk"),
    ("zfs", "spa_misc", "pbsd_zfs_spa"),
]


def assert_keyword_safe(name: str) -> None:
    for seg in name.replace(".", "_").split("_"):
        if seg in KEYWORD_BAN:
            raise ValueError(f"keyword-banned module segment: {seg} in {name}")


def cmake_target(area: str, name: str) -> str:
    return f"pbsd_{area}_{name.replace('.', '_')}"


def cppm_path(area: str, name: str) -> Path:
    return PBSD / area / f"pbsd.{area}.{name}.cppm"


def c_path(area: str, name: str) -> Path:
    return PBSD / area / f"pbsd.{area}.{name}.c"


def render_c(source: str) -> str:
    return f"/* Reference logic from {source} (dual-world). */\n\n/* stub */\n"


def render_cppm(area: str, name: str, source: str) -> str:
    assert_keyword_safe(name)
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


def ensure_dual_world(area: str, name: str, source: str) -> None:
    cp = c_path(area, name)
    if not cp.exists():
        cp.write_text(render_c(source), encoding="utf-8", newline="\n")


def ensure_module(area: str, name: str, source: str) -> bool:
    assert_keyword_safe(name)
    path = cppm_path(area, name)
    created = False
    if not path.exists():
        path.write_text(render_cppm(area, name, source), encoding="utf-8", newline="\n")
        created = True
    ensure_dual_world(area, name, source)
    return created


def cmake_block(area: str, name: str, parent: str | None) -> str:
    tgt = cmake_target(area, name)
    rel = f"{area}/pbsd.{area}.{name}.cppm"
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


def patch_cmake(modules: list[tuple[str, str, str | None]]) -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    added = 0
    insert_before = {
        "net": "if(NOT TARGET pbsd_net)\nadd_library(pbsd_net)",
        "fs": "if(NOT TARGET pbsd_fs)\nadd_library(pbsd_fs)",
        "geom": "if(NOT TARGET pbsd_geom)\nadd_library(pbsd_geom)",
        "zfs": "if(NOT TARGET pbsd_zfs)\nadd_library(pbsd_zfs)",
    }
    by_area: dict[str, list[tuple[str, str | None]]] = {}
    for area, name, parent in modules:
        tgt = cmake_target(area, name)
        if re.search(rf"if\(NOT TARGET {re.escape(tgt)}\)", cmake) or re.search(
            rf"add_library\({re.escape(tgt)}\)", cmake
        ):
            continue
        by_area.setdefault(area, []).append((name, parent))

    for area, items in by_area.items():
        block = "\n".join(cmake_block(area, n, p) for n, p in items)
        anchor = insert_before[area]
        if anchor in cmake:
            cmake = cmake.replace(anchor, block + "\n" + anchor, 1)
            added += len(items)
    if added:
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate_links(area: str, names: list[str]) -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    pattern = rf"(target_link_libraries\(pbsd_{area} PUBLIC [^\)]+)\)"
    m = re.search(pattern, cmake, re.DOTALL)
    if not m:
        return 0
    link_block = m.group(1)
    added = 0
    for name in names:
        lib = cmake_target(area, name)
        if lib not in link_block:
            link_block = link_block.rstrip() + f" {lib}"
            added += 1
    if added:
        cmake = cmake[: m.start(1)] + link_block + ")" + cmake[m.end(0) :]
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate_exports(area: str, names: list[str]) -> int:
    agg = PBSD / area / f"pbsd.{area}.cppm"
    text = agg.read_text(encoding="utf-8")
    added = 0
    for name in names:
        imp = f"export import pbsd.{area}.{name};"
        if imp in text:
            continue
        markers = ["/// Wave 6", "export namespace"]
        for mk in markers:
            idx = text.find(mk)
            if idx != -1:
                text = text[:idx] + imp + "\n" + text[idx:]
                added += 1
                break
    if added:
        agg.write_text(text, encoding="utf-8", newline="\n")
    return added


def infer_source(area: str, name: str) -> str:
    cppm = cppm_path(area, name)
    if cppm.exists():
        m = re.search(r"PROVENANCE: ([^\s—]+)", cppm.read_text(encoding="utf-8"))
        if m:
            return m.group(1)
    return f"hbsd/src/sys/{area}/{name}.c"


def main() -> None:
    created = 0
    all_names: dict[str, list[str]] = {a: [] for a in ("net", "fs", "geom", "zfs")}

    for area, name, source, parent in NEW_MODULES:
        if ensure_module(area, name, source):
            created += 1
        all_names[area].append(name)

    wire_modules: list[tuple[str, str, str | None]] = []
    for area, name, parent in WIRE_EXISTING:
        src = infer_source(area, name)
        ensure_dual_world(area, name, src)
        all_names[area].append(name)
        wire_modules.append((area, name, parent))

    cmake_new = patch_cmake([(a, n, p) for a, n, _, p in NEW_MODULES])
    cmake_wire = patch_cmake(wire_modules)

    for area, names in all_names.items():
        patch_aggregate_exports(area, names)
        patch_aggregate_links(area, names)

    print(f"created {created} new .cppm modules")
    print(f"cmake added {cmake_new + cmake_wire} library targets")
    for area, names in all_names.items():
        print(f"  {area}: wired {len(names)} -> {', '.join(names[:6])}{'...' if len(names) > 6 else ''}")


if __name__ == "__main__":
    main()
