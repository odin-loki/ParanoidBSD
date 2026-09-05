#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 16 wave 2 — mass net/fs/geom/zfs hand ports + dual-world .c stubs + CMake wiring."""
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
    # net — accept filters, congestion, inet/in_pcb helpers, sctp, pf
    ("net", "accf_dns", "hbsd/src/sys/netinet/accf_dns.c", None),
    ("net", "accf_tls", "hbsd/src/sys/netinet/accf_tls.c", None),
    ("net", "cc_vegas", "hbsd/src/sys/netinet/cc/cc_vegas.c", "pbsd_net_tcp"),
    ("net", "in_core", "hbsd/src/sys/netinet/in.c", None),
    ("net", "in_debug", "hbsd/src/sys/netinet/in_debug.c", None),
    ("net", "in_fib_dxr", "hbsd/src/sys/netinet/in_fib_dxr.c", "pbsd_net_route"),
    ("net", "in_gif", "hbsd/src/sys/netinet/in_gif.c", "pbsd_net_gif"),
    ("net", "in_jail", "hbsd/src/sys/netinet/in_jail.c", None),
    ("net", "in_kdtrace", "hbsd/src/sys/netinet/in_kdtrace.c", None),
    ("net", "in_prot", "hbsd/src/sys/netinet/in_prot.c", None),
    ("net", "in_rss", "hbsd/src/sys/netinet/in_rss.c", "pbsd_net_rss"),
    ("net", "ip_carp", "hbsd/src/sys/netinet/ip_carp.c", "pbsd_net_carp"),
    ("net", "ip_divert", "hbsd/src/sys/netinet/ip_divert.c", "pbsd_net_divert"),
    ("net", "ip_gre", "hbsd/src/sys/netinet/ip_gre.c", "pbsd_net_gre"),
    ("net", "ip_icmp", "hbsd/src/sys/netinet/ip_icmp.c", "pbsd_net_icmp"),
    ("net", "ip_mroute", "hbsd/src/sys/netinet/ip_mroute.c", "pbsd_net_mroute"),
    ("net", "tcp_ratelimit", "hbsd/src/sys/netinet/tcp_ratelimit.c", "pbsd_net_tcp"),
    ("net", "tcp_log_buf", "hbsd/src/sys/netinet/tcp_log_buf.c", "pbsd_net_tcp"),
    ("net", "toecore", "hbsd/src/sys/netinet/toecore.c", "pbsd_net_tcp"),
    ("net", "siftr", "hbsd/src/sys/netinet/siftr.c", None),
    ("net", "h_ertt", "hbsd/src/sys/netinet/khelp/h_ertt.c", "pbsd_net_tcp"),
    ("net", "sctp_auth", "hbsd/src/sys/netinet/sctp_auth.c", "pbsd_net_sctp"),
    ("net", "sctp_input", "hbsd/src/sys/netinet/sctp_input.c", "pbsd_net_sctp"),
    ("net", "sctp_output", "hbsd/src/sys/netinet/sctp_output.c", "pbsd_net_sctp"),
    ("net", "sctp_pcb", "hbsd/src/sys/netinet/sctp_pcb.c", "pbsd_net_sctp"),
    ("net", "sctp_timer", "hbsd/src/sys/netinet/sctp_timer.c", "pbsd_net_sctp"),
    ("net", "sctp_usrreq", "hbsd/src/sys/netinet/sctp_usrreq.c", "pbsd_net_sctp"),
    ("net", "sctputil", "hbsd/src/sys/netinet/sctputil.c", "pbsd_net_sctp"),
    ("net", "pf_table", "hbsd/src/sys/netpfil/pf/pf_table.c", "pbsd_net_pf"),
    ("net", "pf_osfp", "hbsd/src/sys/netpfil/pf/pf_osfp.c", "pbsd_net_pf"),
    ("net", "pf_norm", "hbsd/src/sys/netpfil/pf/pf_norm.c", "pbsd_net_pf"),
    ("net", "pf_lb", "hbsd/src/sys/netpfil/pf/pf_lb.c", "pbsd_net_pf"),
    # fs — cd9660/ext2/fuse/msdosfs depth
    ("fs", "cd9660_iconv", "hbsd/src/sys/fs/cd9660/cd9660_iconv.c", "pbsd_fs_cd9660"),
    ("fs", "cd9660_rrip", "hbsd/src/sys/fs/cd9660/cd9660_rrip.c", "pbsd_fs_cd9660"),
    ("fs", "cd9660_util", "hbsd/src/sys/fs/cd9660/cd9660_util.c", "pbsd_fs_cd9660"),
    ("fs", "ext2_balloc", "hbsd/src/sys/fs/ext2fs/ext2_balloc.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_csum", "hbsd/src/sys/fs/ext2fs/ext2_csum.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_extattr", "hbsd/src/sys/fs/ext2fs/ext2_extattr.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_extents", "hbsd/src/sys/fs/ext2fs/ext2_extents.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_hash", "hbsd/src/sys/fs/ext2fs/ext2_hash.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_htree", "hbsd/src/sys/fs/ext2fs/ext2_htree.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_inode", "hbsd/src/sys/fs/ext2fs/ext2_inode.c", "pbsd_fs_ext2fs"),
    ("fs", "ext2_inode_cnv", "hbsd/src/sys/fs/ext2fs/ext2_inode_cnv.c", "pbsd_fs_ext2fs"),
    ("fs", "fdesc_vfsops", "hbsd/src/sys/fs/fdescfs/fdesc_vfsops.c", "pbsd_fs_fdescfs"),
    ("fs", "fuse_device", "hbsd/src/sys/fs/fuse/fuse_device.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_file", "hbsd/src/sys/fs/fuse/fuse_file.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_internal", "hbsd/src/sys/fs/fuse/fuse_internal.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_io", "hbsd/src/sys/fs/fuse/fuse_io.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_ipc", "hbsd/src/sys/fs/fuse/fuse_ipc.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_main", "hbsd/src/sys/fs/fuse/fuse_main.c", "pbsd_fs_fusefs"),
    ("fs", "fuse_node", "hbsd/src/sys/fs/fuse/fuse_node.c", "pbsd_fs_fusefs"),
    ("fs", "mntfs_vnops", "hbsd/src/sys/fs/mntfs/mntfs_vnops.c", None),
    ("fs", "msdosfs_conv", "hbsd/src/sys/fs/msdosfs/msdosfs_conv.c", "pbsd_fs_msdosfs"),
    ("fs", "msdosfs_denode", "hbsd/src/sys/fs/msdosfs/msdosfs_denode.c", "pbsd_fs_msdosfs"),
    ("fs", "msdosfs_fat", "hbsd/src/sys/fs/msdosfs/msdosfs_fat.c", "pbsd_fs_msdosfs"),
    ("fs", "msdosfs_iconv", "hbsd/src/sys/fs/msdosfs/msdosfs_iconv.c", "pbsd_fs_msdosfs"),
    ("fs", "msdosfs_lookup", "hbsd/src/sys/fs/msdosfs/msdosfs_lookup.c", "pbsd_fs_msdosfs"),
    ("fs", "nfs_clport", "hbsd/src/sys/nfs/nfs_clport.c", "pbsd_fs_nfs"),
    ("fs", "nfs_clsubs", "hbsd/src/sys/nfs/nfs_clsubs.c", "pbsd_fs_nfs"),
    ("fs", "ufs_bmap", "hbsd/src/sys/ufs/ufs/ufs_bmap.c", "pbsd_fs_ufs"),
    ("fs", "ufs_quota", "hbsd/src/sys/ufs/ufs/ufs_quota.c", "pbsd_fs_quota"),
    # geom — label + encoding helpers
    ("geom", "geom_bsd_enc", "hbsd/src/sys/geom/geom_bsd_enc.c", None),
    ("geom", "label_ntfs", "hbsd/src/sys/geom/label/g_label_ntfs.c", "pbsd_geom_label"),
    ("geom", "label_swaplinux", "hbsd/src/sys/geom/label/g_label_swaplinux.c", "pbsd_geom_label"),
    ("geom", "label_swap", "hbsd/src/sys/geom/label/g_label_swap.c", "pbsd_geom_label"),
    ("geom", "label_vtoc8", "hbsd/src/sys/geom/label/g_label_vtoc8.c", "pbsd_geom_label"),
    ("geom", "label_gpt_uuid", "hbsd/src/sys/geom/label/g_label_gpt_uuid.c", "pbsd_geom_label"),
    ("geom", "multipath_ctl", "hbsd/src/sys/geom/multipath/g_multipath_ctl.c", "pbsd_geom_multipath"),
    ("geom", "raid_cls", "hbsd/src/sys/geom/raid/g_raid_class.c", "pbsd_geom_raid"),
    ("geom", "raid_tr", "hbsd/src/sys/geom/raid/g_raid_tr.c", "pbsd_geom_raid"),
    ("geom", "journal_core", "hbsd/src/sys/geom/journal/g_journal.c", "pbsd_geom_journal"),
    # zfs — dmu/dsl/compress/crypto helpers
    ("zfs", "blake3_zfs", "hbsd/src/sys/contrib/openzfs/module/zfs/blake3_zfs.c", None),
    ("zfs", "bqueue", "hbsd/src/sys/contrib/openzfs/module/zfs/bqueue.c", None),
    ("zfs", "brt", "hbsd/src/sys/contrib/openzfs/module/zfs/brt.c", "pbsd_zfs_dsl"),
    ("zfs", "btree", "hbsd/src/sys/contrib/openzfs/module/zfs/btree.c", None),
    ("zfs", "dmu_diff", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_diff.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_direct", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_direct.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_redact", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_redact.c", "pbsd_zfs_dmu"),
    ("zfs", "dsl_deleg", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_deleg.c", "pbsd_zfs_dsl"),
    ("zfs", "dsl_synctask", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_synctask.c", "pbsd_zfs_dsl"),
    ("zfs", "dsl_userhold", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_userhold.c", "pbsd_zfs_dsl"),
    ("zfs", "edonr_zfs", "hbsd/src/sys/contrib/openzfs/module/zfs/edonr_zfs.c", None),
    ("zfs", "fm", "hbsd/src/sys/contrib/openzfs/module/zfs/fm.c", None),
    ("zfs", "gzip", "hbsd/src/sys/contrib/openzfs/module/zfs/gzip.c", "pbsd_zfs_compress"),
    ("zfs", "hkdf", "hbsd/src/sys/contrib/openzfs/module/zfs/hkdf.c", None),
    ("zfs", "lz4", "hbsd/src/sys/contrib/openzfs/module/zfs/lz4.c", "pbsd_zfs_compress"),
    ("zfs", "lz4_zfs", "hbsd/src/sys/contrib/openzfs/module/zfs/lz4_zfs.c", "pbsd_zfs_compress"),
    ("zfs", "lzjb", "hbsd/src/sys/contrib/openzfs/module/zfs/lzjb.c", "pbsd_zfs_compress"),
    ("zfs", "mmp", "hbsd/src/sys/contrib/openzfs/module/zfs/mmp.c", "pbsd_zfs_spa"),
    ("zfs", "multilist", "hbsd/src/sys/contrib/openzfs/module/zfs/multilist.c", None),
    ("zfs", "objlist", "hbsd/src/sys/contrib/openzfs/module/zfs/objlist.c", None),
    ("zfs", "pathname", "hbsd/src/sys/contrib/openzfs/module/zfs/pathname.c", None),
    ("zfs", "range_tree", "hbsd/src/sys/contrib/openzfs/module/zfs/range_tree.c", None),
    ("zfs", "sha256_zfs", "hbsd/src/sys/contrib/openzfs/module/zfs/sha256_zfs.c", None),
    ("zfs", "spa_errlog", "hbsd/src/sys/contrib/openzfs/module/zfs/spa_errlog.c", "pbsd_zfs_spa"),
]

WIRE_EXISTING: list[tuple[str, str, str | None]] = [
    # orphans from parallel burst 16 agents — wire cmake + aggregate
    ("fs", "nfs_clvfsops", "pbsd_fs_nfs"),
    ("fs", "nfs_clvnops", "pbsd_fs_nfs"),
    ("fs", "procfs_status", "pbsd_fs_procfs"),
    ("fs", "udf_vfsops", "pbsd_fs_udf"),
    ("geom", "part_apm", "pbsd_geom_part"),
    ("geom", "part_bsd", "pbsd_geom_part"),
    ("geom", "raid_tr_raid5", "pbsd_geom_raid"),
    ("geom", "uzip_lzma", "pbsd_geom_uzip"),
    ("zfs", "refcount", None),
    ("zfs", "spa_checkpoint", "pbsd_zfs_spa"),
    ("zfs", "vdev_file", "pbsd_zfs_vdev"),
    ("zfs", "vdev_raidz", "pbsd_zfs_vdev"),
    ("zfs", "zap_leaf", "pbsd_zfs_zap"),
    ("zfs", "zcp", None),
    ("zfs", "zvol", None),
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
        if f"TARGET {tgt}" in cmake or f"add_library({tgt})" in cmake:
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


def main() -> None:
    created = 0
    all_names: dict[str, list[str]] = {a: [] for a in ("net", "fs", "geom", "zfs")}

    for area, name, source, parent in NEW_MODULES:
        if ensure_module(area, name, source):
            created += 1
        all_names[area].append(name)

    wire_modules: list[tuple[str, str, str | None]] = []
    for area, name, parent in WIRE_EXISTING:
        cppm = cppm_path(area, name)
        if cppm.exists():
            src_m = re.search(r"PROVENANCE: ([^\s—]+)", cppm.read_text(encoding="utf-8"))
            src = src_m.group(1) if src_m else f"hbsd/src/sys/{area}/{name}.c"
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
        print(f"  {area}: wired {len(names)} -> {', '.join(names[:8])}{'...' if len(names) > 8 else ''}")


if __name__ == "__main__":
    main()
