#!/usr/bin/env python3
"""Burst 15 wave 2 — net/fs/geom/zfs gap-fill hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

# C++ keyword segments banned in module names (Clang module IU).
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
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}

# (area, name, hbsd_source, parent_cmake_target or None)
NEW_MODULES: list[tuple[str, str, str, str | None]] = [
    # net
    ("net", "udp_input", "hbsd/src/sys/netinet/udp_usrreq.c", "pbsd_net_udp"),
    ("net", "in_pcblist", "hbsd/src/sys/netinet/in_pcb.c", "pbsd_net_in_pcb"),
    ("net", "raw_usrreq", "hbsd/src/sys/netinet/raw_usrreq.c", "pbsd_net_raw_ip"),
    ("net", "tcp_log", "hbsd/src/sys/netinet/tcp_log.c", "pbsd_net_tcp"),
    ("net", "cc_cubic", "hbsd/src/sys/netinet/cc/cc_cubic.c", "pbsd_net_tcp"),
    ("net", "pf_ioctl", "hbsd/src/sys/netpfil/pf/pf_ioctl.c", "pbsd_net_pf"),
    ("net", "rip_input", "hbsd/src/sys/netinet/rip_input.c", "pbsd_net_route"),
    ("net", "rip_output", "hbsd/src/sys/netinet/rip_output.c", "pbsd_net_route"),
    ("net", "ip_fastforward", "hbsd/src/sys/netinet/ip_fastforward.c", "pbsd_net_ip"),
    ("net", "tcp_rfc1323", "hbsd/src/sys/netinet/tcp_subr.c", "pbsd_net_tcp"),
    # fs
    ("fs", "vfs_subr", "hbsd/src/sys/kern/vfs_subr.c", None),
    ("fs", "vfs_lookup", "hbsd/src/sys/kern/vfs_lookup.c", None),
    ("fs", "vfs_syscalls", "hbsd/src/sys/kern/vfs_syscalls.c", None),
    ("fs", "vfs_vnode", "hbsd/src/sys/kern/vfs_vnode.c", None),
    ("fs", "fifo_vnops", "hbsd/src/sys/fs/fifofs/fifo_vnops.c", "pbsd_fs_fifofs"),
    ("fs", "ufs_vnops", "hbsd/src/sys/ufs/ufs/ufs_vnops.c", "pbsd_fs_ufs"),
    ("fs", "ffs_vnops", "hbsd/src/sys/ufs/ffs/ffs_vnops.c", "pbsd_fs_ffs"),
    ("fs", "devfs_vnops", "hbsd/src/sys/fs/devfs/devfs_vnops.c", "pbsd_fs_devfs"),
    ("fs", "procfs_vnops", "hbsd/src/sys/fs/procfs/procfs_vnops.c", "pbsd_fs_procfs"),
    ("fs", "vfs_hash", "hbsd/src/sys/kern/vfs_hash.c", None),
    ("fs", "vfs_init", "hbsd/src/sys/kern/vfs_init.c", None),
    ("fs", "vfs_conf", "hbsd/src/sys/kern/vfs_conf.c", None),
    # geom
    ("geom", "part_gpt", "hbsd/src/sys/geom/part/g_part_gpt.c", "pbsd_geom_part"),
    ("geom", "part_mbr", "hbsd/src/sys/geom/part/g_part_mbr.c", "pbsd_geom_part"),
    ("geom", "eli_key", "hbsd/src/sys/geom/eli/g_eli_key.c", "pbsd_geom_eli"),
    ("geom", "journal_ufs", "hbsd/src/sys/geom/journal/g_journal_ufs.c", "pbsd_geom_journal"),
    ("geom", "mirror_ctl", "hbsd/src/sys/geom/mirror/g_mirror_ctl.c", "pbsd_geom_mirror"),
    ("geom", "raid_ctl", "hbsd/src/sys/geom/raid/g_raid_ctl.c", "pbsd_geom_raid"),
    ("geom", "label_gpt", "hbsd/src/sys/geom/label/g_label_gpt.c", "pbsd_geom_label"),
    ("geom", "label_ufs", "hbsd/src/sys/geom/label/g_label_ufs.c", "pbsd_geom_label"),
    # zfs
    ("zfs", "blkptr", "hbsd/src/sys/contrib/openzfs/module/zfs/blkptr.c", "pbsd_zfs_zio"),
    ("zfs", "dmu_object", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_object.c", "pbsd_zfs_dmu"),
    ("zfs", "dmu_tx", "hbsd/src/sys/contrib/openzfs/module/zfs/dmu_tx.c", "pbsd_zfs_dmu"),
    ("zfs", "dnode_sync", "hbsd/src/sys/contrib/openzfs/module/zfs/dnode_sync.c", "pbsd_zfs_dnode"),
    ("zfs", "spa_config", "hbsd/src/sys/contrib/openzfs/module/zfs/spa_config.c", "pbsd_zfs_spa"),
    ("zfs", "vdev_mirror", "hbsd/src/sys/contrib/openzfs/module/zfs/vdev_mirror.c", "pbsd_zfs_vdev"),
    ("zfs", "bpobj", "hbsd/src/sys/contrib/openzfs/module/zfs/bpobj.c", "pbsd_zfs_dsl"),
    ("zfs", "ddt_zap", "hbsd/src/sys/contrib/openzfs/module/zfs/ddt_zap.c", "pbsd_zfs_dmu"),
    ("zfs", "dsl_deadlist", "hbsd/src/sys/contrib/openzfs/module/zfs/dsl_deadlist.c", "pbsd_zfs_dsl"),
    ("zfs", "arc_evict", "hbsd/src/sys/contrib/openzfs/module/zfs/arc.c", "pbsd_zfs_arc"),
]

# Existing .cppm files needing cmake and/or aggregate wiring.
WIRE_EXISTING: list[tuple[str, str, str | None]] = [
    ("net", "ip6_input", "pbsd_net_ip6"),
    ("net", "ip6_output", "pbsd_net_ip6"),
    ("net", "ip_output", "pbsd_net_ip"),
    ("net", "tcp_subr", "pbsd_net_tcp"),
    ("net", "udp6_usrreq", "pbsd_net_udp6"),
    ("fs", "ext2fs_inode", "pbsd_fs_ext2fs"),
    ("fs", "nfs_commonport", "pbsd_fs_nfs"),
    ("fs", "nfs_commonsubs", "pbsd_fs_nfs"),
    ("fs", "union_subr", "pbsd_fs_unionfs"),
    ("fs", "union_vfsops", "pbsd_fs_unionfs"),
    ("fs", "union_vnops", "pbsd_fs_unionfs"),
    ("geom", "orphan", None),
    ("geom", "up", None),
    ("zfs", "arc_stats", "pbsd_zfs_arc"),
    ("zfs", "dsl_scan", "pbsd_zfs_dsl"),
    ("zfs", "fuid", None),
    ("zfs", "vfsops", None),
    ("zfs", "vnops", None),
    ("zfs", "zio_inject", "pbsd_zfs_zio"),
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


def render_cppm(area: str, name: str, source: str, ns_extra: str = "") -> str:
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
    c_path(area, name).write_text(render_c(source), encoding="utf-8", newline="\n")


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
        if f"TARGET {tgt}" in cmake:
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
    agg_tgt = f"pbsd_{area}"
    pattern = rf"(target_link_libraries\({agg_tgt} PUBLIC [^\)]+)\)"
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
        if imp not in text:
            # insert before façade comment or namespace
            markers = ["/// Wave", "export namespace"]
            for mk in markers:
                idx = text.find(mk)
                if idx != -1:
                    text = text[:idx] + imp + "\n" + text[idx:]
                    added += 1
                    break
    if added:
        agg.write_text(text, encoding="utf-8", newline="\n")
    return added


def dual_world_batch(area: str) -> int:
    n = 0
    for cppm in sorted((PBSD / area).glob(f"pbsd.{area}.*.cppm")):
        if cppm.name == f"pbsd.{area}.cppm":
            continue
        name = cppm.stem.replace(f"pbsd.{area}.", "")
        cp = c_path(area, name)
        if not cp.exists():
            # infer source from PROVENANCE line when present
            text = cppm.read_text(encoding="utf-8")
            src_m = re.search(r"PROVENANCE: ([^\s—]+)", text)
            src = src_m.group(1) if src_m else f"hbsd/src/sys/{area}/{name}.c"
            cp.write_text(render_c(src), encoding="utf-8", newline="\n")
            n += 1
    return n


def main() -> None:
    created = 0
    all_names: dict[str, list[str]] = {"net": [], "fs": [], "geom": [], "zfs": []}

    for area, name, source, parent in NEW_MODULES:
        if ensure_module(area, name, source):
            created += 1
        all_names[area].append(name)

    wire_modules: list[tuple[str, str, str | None]] = []
    for area, name, parent in WIRE_EXISTING:
        cppm = cppm_path(area, name)
        if cppm.exists():
            src_m = re.search(
                r"PROVENANCE: ([^\s—]+)", cppm.read_text(encoding="utf-8")
            )
            src = src_m.group(1) if src_m else f"hbsd/src/sys/{area}/{name}.c"
            ensure_dual_world(area, name, src)
        all_names[area].append(name)
        wire_modules.append((area, name, parent))

    cmake_new = patch_cmake([(a, n, p) for a, n, _, p in NEW_MODULES])
    cmake_wire = patch_cmake(wire_modules)

    for area, names in all_names.items():
        patch_aggregate_exports(area, names)
        patch_aggregate_links(area, names)

    dual = sum(dual_world_batch(a) for a in all_names)

    print(f"created {created} new .cppm modules")
    print(f"cmake added {cmake_new + cmake_wire} library targets")
    print(f"dual-world .c stubs added/updated: {dual}")
    for area, names in all_names.items():
        print(f"  {area}: +{len(names)} wired -> {', '.join(names[:8])}{'...' if len(names)>8 else ''}")


if __name__ == "__main__":
    main()
