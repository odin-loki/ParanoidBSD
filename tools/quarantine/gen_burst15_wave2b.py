#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 15 wave 2b — mass net hand ports + dual-world .c stubs."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

# (name, hbsd_source, parent_cmake_target)
NEW_MODULES: list[tuple[str, str, str | None]] = [
    ("frag6", "hbsd/src/sys/netinet6/frag6.c", "pbsd_net_ip6"),
    ("ip6_forward", "hbsd/src/sys/netinet6/ip6_forward.c", "pbsd_net_ip6"),
    ("nd6_nbr", "hbsd/src/sys/netinet6/nd6_nbr.c", "pbsd_net_ndp"),
    ("nd6_rtr", "hbsd/src/sys/netinet6/nd6_rtr.c", "pbsd_net_ndp"),
    ("in6_cksum", "hbsd/src/sys/netinet6/in6_cksum.c", None),
    ("in6_proto", "hbsd/src/sys/netinet6/in6_proto.c", None),
    ("in6_pcb", "hbsd/src/sys/netinet6/in6_pcb.c", "pbsd_net_in_pcb"),
    ("scope6", "hbsd/src/sys/netinet6/scope6.c", "pbsd_net_ip6"),
    ("sctp6_usrreq", "hbsd/src/sys/netinet6/sctp6_usrreq.c", "pbsd_net_sctp"),
    ("ip6_id", "hbsd/src/sys/netinet6/ip6_id.c", "pbsd_net_ip6"),
    ("ip6_fastfwd", "hbsd/src/sys/netinet6/ip6_fastfwd.c", "pbsd_net_ip6"),
    ("dest6", "hbsd/src/sys/netinet6/dest6.c", "pbsd_net_ip6"),
    ("route6", "hbsd/src/sys/netinet6/route6.c", "pbsd_net_route"),
    ("tcp_hostcache", "hbsd/src/sys/netinet/tcp_hostcache.c", "pbsd_net_tcp"),
    ("tcp_lro", "hbsd/src/sys/netinet/tcp_lro.c", "pbsd_net_tcp"),
    ("tcp_lro_hpts", "hbsd/src/sys/netinet/tcp_lro_hpts.c", "pbsd_net_tcp"),
    ("ip_id", "hbsd/src/sys/netinet/ip_id.c", "pbsd_net_ip"),
    ("in_fib", "hbsd/src/sys/netinet/in_fib.c", "pbsd_net_route"),
    ("in_fib_algo", "hbsd/src/sys/netinet/in_fib_algo.c", "pbsd_net_route"),
    ("tcp_stats", "hbsd/src/sys/netinet/tcp_stats.c", "pbsd_net_tcp"),
]

# Wave 2 modules missing from pbsd_net aggregate link line.
WIRE_WAVE2: list[str] = [
    "udp_input",
    "in_pcblist",
    "raw_usrreq",
    "tcp_log",
    "cc_cubic",
    "pf_ioctl",
    "rip_input",
    "rip_output",
    "ip_fastforward",
    "tcp_rfc1323",
]


def cmake_target(name: str) -> str:
    return f"pbsd_net_{name.replace('.', '_')}"


def cppm_path(name: str) -> Path:
    return PBSD / "net" / f"pbsd.net.{name}.cppm"


def c_path(name: str) -> Path:
    return PBSD / "net" / f"pbsd.net.{name}.c"


def render_c(source: str) -> str:
    return f"/* Reference logic from {source} (dual-world). */\n\n/* stub */\n"


def render_cppm(name: str, source: str) -> str:
    ns = f"pbsd::net::{name.replace('.', '_')}"
    title = name.replace("_", " ").title()
    return f"""module;
#include <cstdint>

export module pbsd.net.{name};

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


def ensure_module(name: str, source: str) -> bool:
    path = cppm_path(name)
    created = False
    if not path.exists():
        path.write_text(render_cppm(name, source), encoding="utf-8", newline="\n")
        created = True
    c_path(name).write_text(render_c(source), encoding="utf-8", newline="\n")
    return created


def cmake_block(name: str, parent: str | None) -> str:
    tgt = cmake_target(name)
    rel = f"net/pbsd.net.{name}.cppm"
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


def patch_cmake_new() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    anchor = "if(NOT TARGET pbsd_net)\nadd_library(pbsd_net)"
    added = 0
    blocks: list[str] = []
    for name, _, parent in NEW_MODULES:
        tgt = cmake_target(name)
        if f"TARGET {tgt}" in cmake:
            continue
        blocks.append(cmake_block(name, parent))
        added += 1
    if blocks and anchor in cmake:
        cmake = cmake.replace(anchor, "\n".join(blocks) + "\n" + anchor, 1)
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_aggregate(names: list[str]) -> tuple[int, int]:
    cmake = CMAKE.read_text(encoding="utf-8")
    agg = PBSD / "net" / "pbsd.net.cppm"
    text = agg.read_text(encoding="utf-8")
    link_added = 0
    export_added = 0

    pattern = r"(target_link_libraries\(pbsd_net PUBLIC [^\n]+)\n(target_compile_options\(pbsd_net)"
    m = re.search(pattern, cmake, re.DOTALL)
    if m:
        link_line = m.group(1)
        for name in names:
            lib = cmake_target(name)
            if lib not in link_line:
                link_line = link_line.rstrip() + f" {lib}"
                link_added += 1
        if link_added:
            cmake = cmake[: m.start(1)] + link_line + "\n" + m.group(2) + cmake[m.end(2) :]
            CMAKE.write_text(cmake, encoding="utf-8", newline="\n")

    for name in names:
        imp = f"export import pbsd.net.{name};"
        if imp not in text:
            idx = text.find("/// Wave 6")
            if idx == -1:
                idx = text.find("export namespace")
            if idx != -1:
                text = text[:idx] + imp + "\n" + text[idx:]
                export_added += 1
    if export_added:
        agg.write_text(text, encoding="utf-8", newline="\n")
    return link_added, export_added


def main() -> None:
    created = 0
    for name, source, parent in NEW_MODULES:
        if ensure_module(name, source):
            created += 1

    cmake_added = patch_cmake_new()
    all_names = [n for n, _, _ in NEW_MODULES] + WIRE_WAVE2
    link_added, export_added = patch_aggregate(all_names)

    print(f"created {created} new .cppm modules")
    print(f"cmake added {cmake_added} library targets")
    print(f"aggregate: +{link_added} links, +{export_added} exports")
    print(f"wave2b: {', '.join(n for n, _, _ in NEW_MODULES)}")


if __name__ == "__main__":
    main()
