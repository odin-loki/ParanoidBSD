#!/usr/bin/env python3
"""Wave 7 — 33 hand modules under pbsd/net, fs, geom, zfs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

# (tree, name, provenance, parent_cmake_target or None -> pbsd_core)
MODULES: list[tuple[str, str, str, str | None]] = [
    # net (11)
    ("net", "toecore", "hbsd/src/sys/netinet/toecore.c", "pbsd_net_tcp"),
    ("net", "tcp_ratelimit", "hbsd/src/sys/netinet/tcp_ratelimit.c", "pbsd_net_tcp"),
    ("net", "tcp_log_buf", "hbsd/src/sys/netinet/tcp_log_buf.c", "pbsd_net_tcp"),
    ("net", "siftr", "hbsd/src/sys/netinet/siftr.c", None),
    ("net", "accf_dns", "hbsd/src/sys/netinet/accf_dns.c", None),
    ("net", "accf_tls", "hbsd/src/sys/netinet/accf_tls.c", None),
    ("net", "in_debug", "hbsd/src/sys/netinet/in_debug.c", None),
    ("net", "in_jail", "hbsd/src/sys/netinet/in_jail.c", None),
    ("net", "in_rss", "hbsd/src/sys/netinet/in_rss.c", "pbsd_net_rss"),
    ("net", "sctp_pcb", "hbsd/src/sys/netinet/sctp_pcb.c", "pbsd_net_sctp"),
    ("net", "sctp_input", "hbsd/src/sys/netinet/sctp_input.c", "pbsd_net_sctp"),
    # fs (8)
    ("fs", "ext2_inode", "hbsd/src/sys/fs/ext2fs/ext2_inode.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_balloc", "hbsd/src/sys/fs/ext2fs/ext2_balloc.c", "pbsd_fs_ext2fs"),
    ("fs", "msdosfs_lookup", "hbsd/src/sys/fs/msdosfs/msdosfs_lookup.c", "pbsd_fs_msdosfs"),
    ("fs", "nfs_clvnops", "hbsd/src/sys/fs/nfs/nfs_clvnops.c", "pbsd_fs_nfs"),
    ("fs", "nfs_clvfsops", "hbsd/src/sys/fs/nfs/nfs_clvfsops.c", "pbsd_fs_nfs"),
    ("fs", "fuse_node", "hbsd/src/sys/fs/fuse/fuse_node.c", "pbsd_fs_fusefs"),
    ("fs", "procfs_status", "hbsd/src/sys/fs/procfs/procfs_status.c", "pbsd_fs_procfs"),
    ("fs", "udf_vfsops", "hbsd/src/sys/fs/udf/udf_vfsops.c", "pbsd_fs_udf"),
    # geom (6)
    ("geom", "part_apm", "hbsd/src/sys/geom/part/g_part_apm.c", "pbsd_geom_part"),
    ("geom", "part_bsd", "hbsd/src/sys/geom/part/g_part_bsd.c", "pbsd_geom_part"),
    ("geom", "raid_tr_raid5", "hbsd/src/sys/geom/raid/tr_raid5.c", "pbsd_geom_raid"),
    ("geom", "geom_bsd_enc", "hbsd/src/sys/geom/geom_bsd_enc.c", "pbsd_geom_bsd"),
    ("geom", "label_ntfs", "hbsd/src/sys/geom/label/g_label_ntfs.c", "pbsd_geom_label"),
    ("geom", "uzip_lzma", "hbsd/src/sys/geom/uzip/g_uzip_lzma.c", "pbsd_geom_uzip"),
    # zfs (8)
    ("zfs", "range_tree", "hbsd/src/sys/contrib/openzfs/module/zfs/range_tree.c", None),
    ("zfs", "refcount", "hbsd/src/sys/contrib/openzfs/module/zfs/refcount.c", None),
    ("zfs", "vdev_raidz", "hbsd/src/sys/contrib/openzfs/module/zfs/vdev_raidz.c", "pbsd_zfs_vdev"),
    ("zfs", "zvol", "hbsd/src/sys/contrib/openzfs/module/zfs/zvol.c", "pbsd_zfs_dmu"),
    ("zfs", "zap_leaf", "hbsd/src/sys/contrib/openzfs/module/zfs/zap_leaf.c", "pbsd_zfs_zap"),
    ("zfs", "zcp", "hbsd/src/sys/contrib/openzfs/module/zfs/zcp.c", "pbsd_zfs_dsl"),
    ("zfs", "spa_checkpoint", "hbsd/src/sys/contrib/openzfs/module/zfs/spa_checkpoint.c", "pbsd_zfs_spa"),
    ("zfs", "vdev_file", "hbsd/src/sys/contrib/openzfs/module/zfs/vdev_file.c", "pbsd_zfs_vdev"),
]

AGG_MARKERS = {
    "net": ("/// Wave 6 — C++23 network stack façade", "pbsd.net"),
    "fs": ("/// Wave 6 — VFS / ZFS boundary façade.", "pbsd.fs"),
    "geom": ("/// Wave 6 — GEOM provider/consumer façade", "pbsd.geom"),
    "zfs": ("/// Wave 6 — OpenZFS port track", "pbsd.zfs"),
}

CMAKE_INSERT_BEFORE = {
    "net": "if(NOT TARGET pbsd_net)\nadd_library(pbsd_net)",
    "fs": "if(NOT TARGET pbsd_fs)\nadd_library(pbsd_fs)",
    "geom": "if(NOT TARGET pbsd_geom)\nadd_library(pbsd_geom)",
    "zfs": "if(NOT TARGET pbsd_zfs)\nadd_library(pbsd_zfs)",
}


def mod_prefix(tree: str) -> str:
    return f"pbsd.{tree}"


def cmake_target(tree: str, name: str) -> str:
    return f"pbsd_{tree}_{name.replace('.', '_')}"


def cppm_path(tree: str, name: str) -> Path:
    return PBSD / tree / f"{mod_prefix(tree)}.{name}.cppm"


def render_cppm(tree: str, name: str, source: str) -> str:
    mod = f"{mod_prefix(tree)}.{name}"
    ns = f"pbsd::{tree}::{name.replace('.', '_')}"
    title = name.replace("_", " ").title()
    return f"""module;
#include <cstdint>

export module {mod};

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


def render_cmake_block(tree: str, name: str) -> str:
    tgt = cmake_target(tree, name)
    rel = f"{tree}/{mod_prefix(tree)}.{name}.cppm"
    parent = next(p for t, n, _, p in MODULES if t == tree and n == name)
    links = "pbsd_core"
    if parent:
        links = f"pbsd_core {parent}"
    return f"""
if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {links})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def patch_aggregate(tree: str, names: list[str]) -> None:
    agg = PBSD / tree / f"{mod_prefix(tree)}.cppm"
    text = agg.read_text(encoding="utf-8")
    marker, prefix = AGG_MARKERS[tree]
    imports = []
    for name in names:
        imp = f"export import {prefix}.{name};"
        if imp not in text:
            imports.append(imp)
    if not imports:
        return
    block = "\n".join(imports) + "\n"
    if marker not in text:
        raise SystemExit(f"marker not found in {agg}")
    text = text.replace(marker, block + marker, 1)
    agg.write_text(text, encoding="utf-8")
    print(f"  patched {agg.name} (+{len(imports)} imports)")


def patch_cmake(tree: str, names: list[str]) -> None:
    text = CMAKE.read_text(encoding="utf-8")
    needle = CMAKE_INSERT_BEFORE[tree]
    if needle not in text:
        raise SystemExit(f"insert marker missing for {tree}")
    blocks = "".join(render_cmake_block(tree, n) for n in names)
    text = text.replace(needle, blocks + needle, 1)

    # append to aggregate target_link_libraries line (last occurrence before endif)
    agg_pat = re.compile(
        rf"(if\(NOT TARGET pbsd_{tree}\)[\s\S]*?target_link_libraries\(pbsd_{tree} PUBLIC[\s\S]*?)(\\)\n(\s*target_compile_options)",
        re.MULTILINE,
    )
    m = agg_pat.search(text)
    if not m:
        raise SystemExit(f"aggregate link line not found for {tree}")
    additions = " ".join(cmake_target(tree, n) for n in names)
    text = text[: m.end(1)] + " " + additions + text[m.end(1) :]
    CMAKE.write_text(text, encoding="utf-8")
    print(f"  patched CMakeLists (+{len(names)} {tree} targets)")


def main() -> None:
    by_tree: dict[str, list[str]] = {}
    created = 0
    skipped = 0
    for tree, name, source, _ in MODULES:
        path = cppm_path(tree, name)
        by_tree.setdefault(tree, []).append(name)
        if path.exists():
            print(f"skip existing {path.relative_to(ROOT)}")
            skipped += 1
            continue
        path.write_text(render_cppm(tree, name, source), encoding="utf-8")
        print(f"write {path.relative_to(ROOT)}")
        created += 1

    for tree, names in by_tree.items():
        patch_aggregate(tree, names)
        patch_cmake(tree, names)

    print(f"\nDone: {created} created, {skipped} skipped, {len(MODULES)} total")


if __name__ == "__main__":
    main()
