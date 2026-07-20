#!/usr/bin/env python3
"""Burst 16 wave 2 — mass kernel + UDA hand ports (dual-world)."""
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
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}

CREATED: list[str] = []
KERNEL_TARGETS: list[tuple[str, str, str]] = []
UDA_TARGETS: list[tuple[str, str, str]] = []

# (area subdir, module stem, hbsd provenance, optional parent dep)
KERNEL_MODULES: list[tuple[str, str, str, str | None]] = [
    # kern_*
    ("kern", "kern_sema", "hbsd/src/sys/kern/kern_sema.c", None),
    ("kern", "kern_shutdown", "hbsd/src/sys/kern/kern_shutdown.c", None),
    ("kern", "kern_devctl", "hbsd/src/sys/kern/kern_devctl.c", "pbsd_kernel_filedesc"),
    ("kern", "kern_mtxpool", "hbsd/src/sys/kern/kern_mtxpool.c", "pbsd_kernel_mutex"),
    ("kern", "kern_rangelock", "hbsd/src/sys/kern/kern_rangelock.c", None),
    ("kern", "kern_priv", "hbsd/src/sys/kern/kern_priv.c", "pbsd_kernel_ucred"),
    ("kern", "kern_hhook", "hbsd/src/sys/kern/kern_hhook.c", None),
    ("kern", "kern_loginclass", "hbsd/src/sys/kern/kern_loginclass.c", None),
    ("kern", "kern_osd", "hbsd/src/sys/kern/kern_osd.c", None),
    ("kern", "kern_pmc", "hbsd/src/sys/kern/kern_pmc.c", None),
    ("kern", "kern_sdt", "hbsd/src/sys/kern/kern_sdt.c", None),
    ("kern", "kern_tc", "hbsd/src/sys/kern/kern_tc.c", None),
    ("kern", "kern_tslog", "hbsd/src/sys/kern/kern_tslog.c", None),
    ("kern", "kern_ubsan", "hbsd/src/sys/kern/kern_ubsan.c", None),
    ("kern", "kern_ffclock", "hbsd/src/sys/kern/kern_ffclock.c", "pbsd_kernel_timekeeping"),
    ("kern", "kern_clocksource", "hbsd/src/sys/kern/kern_clocksource.c", "pbsd_kernel_timekeeping"),
    ("kern", "kern_dump", "hbsd/src/sys/kern/kern_dump.c", None),
    ("kern", "kern_fail", "hbsd/src/sys/kern/kern_fail.c", None),
    ("kern", "kern_jaildesc", "hbsd/src/sys/kern/kern_jaildesc.c", "pbsd_kernel_jail"),
    ("kern", "kern_jailmeta", "hbsd/src/sys/kern/kern_jailmeta.c", "pbsd_kernel_jail"),
    ("kern", "kern_kcov", "hbsd/src/sys/kern/kern_kcov.c", None),
    ("kern", "kern_khelp", "hbsd/src/sys/kern/kern_khelp.c", None),
    ("kern", "kern_kthread", "hbsd/src/sys/kern/kern_kthread.c", "pbsd_handles"),
    ("kern", "kern_ktr", "hbsd/src/sys/kern/kern_ktr.c", "pbsd_kernel_ktrace"),
    ("kern", "kern_alq", "hbsd/src/sys/kern/kern_alq.c", None),
    ("kern", "kern_boottrace", "hbsd/src/sys/kern/kern_boottrace.c", None),
    ("kern", "kern_dtrace", "hbsd/src/sys/kern/kern_dtrace.c", None),
    ("kern", "kern_et", "hbsd/src/sys/kern/kern_et.c", None),
    # subr_*
    ("kern", "subr_clockcalib", "hbsd/src/sys/kern/subr_clockcalib.c", "pbsd_kernel_timekeeping"),
    ("kern", "subr_compressor", "hbsd/src/sys/kern/subr_compressor.c", None),
    ("kern", "subr_efi_map", "hbsd/src/sys/kern/subr_efi_map.c", None),
    ("kern", "subr_atomic64", "hbsd/src/sys/kern/subr_atomic64.c", None),
    ("kern", "subr_asan", "hbsd/src/sys/kern/subr_asan.c", None),
    ("kern", "subr_csan", "hbsd/src/sys/kern/subr_csan.c", None),
    ("kern", "subr_msan", "hbsd/src/sys/kern/subr_msan.c", None),
    ("kern", "subr_coverage", "hbsd/src/sys/kern/subr_coverage.c", None),
    ("kern", "subr_dummy_vdso_tc", "hbsd/src/sys/kern/subr_dummy_vdso_tc.c", "pbsd_kernel_timekeeping"),
    # vm
    ("vm", "vm_pageout", "hbsd/src/sys/vm/vm_pageout.c", "pbsd_kernel_vm_page"),
    ("vm", "vm_mmap", "hbsd/src/sys/vm/vm_mmap.c", "pbsd_kernel_vm_map"),
    ("vm", "vm_init", "hbsd/src/sys/vm/vm_init.c", "pbsd_kernel_vm"),
    ("vm", "vm_glue", "hbsd/src/sys/vm/vm_glue.c", "pbsd_kernel_vm"),
    ("vm", "device_pager", "hbsd/src/sys/vm/device_pager.c", "pbsd_kernel_vm_pager"),
    ("vm", "swap_pager", "hbsd/src/sys/vm/swap_pager.c", "pbsd_kernel_vm_pager"),
    ("vm", "phys_pager", "hbsd/src/sys/vm/phys_pager.c", "pbsd_kernel_vm_pager"),
    ("vm", "sg_pager", "hbsd/src/sys/vm/sg_pager.c", "pbsd_kernel_vm_pager"),
    ("vm", "vnode_pager", "hbsd/src/sys/vm/vnode_pager.c", "pbsd_kernel_vm_pager"),
    ("vm", "vm_domainset", "hbsd/src/sys/vm/vm_domainset.c", "pbsd_kernel_numa"),
    ("vm", "memguard", "hbsd/src/sys/vm/memguard.c", "pbsd_kernel_vm"),
    ("vm", "uma_core", "hbsd/src/sys/vm/uma_core.c", "pbsd_kernel_uma"),
    ("vm", "vm_unix", "hbsd/src/sys/vm/vm_unix.c", "pbsd_kernel_vm"),
    ("vm", "vm_swapout", "hbsd/src/sys/vm/vm_swapout.c", "pbsd_kernel_vm_page"),
    # net helpers
    ("net", "ethersubr", "hbsd/src/sys/net/if_ethersubr.c", "pbsd_kernel_ether"),
    ("net", "llatbl", "hbsd/src/sys/net/if_llatbl.c", "pbsd_kernel_route"),
    ("net", "bpf_filter", "hbsd/src/sys/net/bpf_filter.c", "pbsd_kernel_bpf"),
    ("net", "bridgestp", "hbsd/src/sys/net/bridgestp.c", "pbsd_kernel_bridge"),
    ("net", "debugnet", "hbsd/src/sys/net/debugnet.c", None),
    ("net", "pfil_helpers", "hbsd/src/sys/net/pfil.c", "pbsd_kernel_pfil"),
]

# (stem, upstream, device_class, vendor, device, body snippet)
UDA_MODULES: list[tuple[str, str, str, str, str, str]] = [
    (
        "cfumass",
        "hbsd/src/sys/dev/usb/storage/cfumass.c",
        "Block",
        "0x0000",
        "0x0000",
        """inline constexpr std::uint8_t kCfumassEnable = 0x01;

[[nodiscard]] inline Status validate_lun(unsigned lun) noexcept {
    return lun < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "vte",
        "hbsd/src/sys/dev/vte/if_vte.c",
        "Network",
        "0x0002",
        "0x0003",
        """inline constexpr std::uint32_t kRegMacCtl = 0x0008;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ae",
        "hbsd/src/sys/dev/ae/if_ae.c",
        "Network",
        "0x10ec",
        "0x8168",
        """inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_mii(unsigned reg) noexcept {
    return reg <= 31 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "lge",
        "hbsd/src/sys/dev/lge/if_lge.c",
        "Network",
        "0x11ab",
        "0x4320",
        """inline constexpr std::uint32_t kRegHostCmd = 0x0000;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 4 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "rl",
        "hbsd/src/sys/dev/rl/if_rl.c",
        "Network",
        "0x10ec",
        "0x8139",
        """inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_frame(unsigned len) noexcept {
    return len >= 60 && len <= 1518 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "nge",
        "hbsd/src/sys/dev/nge/if_nge.c",
        "Network",
        "0x10ec",
        "0x8169",
        """inline constexpr std::uint32_t kRegTxPoll = 0x0038;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "my",
        "hbsd/src/sys/dev/my/if_my.c",
        "Network",
        "0x1516",
        "0x0800",
        """inline constexpr std::uint32_t kRegControl = 0x0000;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 32 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "mgb",
        "hbsd/src/sys/dev/mgb/if_mgb.c",
        "Network",
        "0x15ad",
        "0x1533",
        """inline constexpr std::uint32_t kRegMacCtl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "hn",
        "hbsd/src/sys/dev/hyperv/netvsc/if_hn.c",
        "Network",
        "0x1414",
        "0x0001",
        """inline constexpr std::uint32_t kRegVfId = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 64 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ix",
        "hbsd/src/sys/dev/ixgbe/if_ix.c",
        "Network",
        "0x8086",
        "0x10fb",
        """inline constexpr std::uint32_t kRegCtrl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 128 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ixv",
        "hbsd/src/sys/dev/ixgbe/if_ixv.c",
        "Network",
        "0x8086",
        "0x10ed",
        """inline constexpr std::uint32_t kRegCtrl = 0x0000;

[[nodiscard]] inline Status validate_vf(unsigned vf) noexcept {
    return vf < 32 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ipw",
        "hbsd/src/sys/dev/ipw/if_ipw.c",
        "Network",
        "0x8086",
        "0x4220",
        """inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "iwi",
        "hbsd/src/sys/dev/iwi/if_iwi.c",
        "Network",
        "0x8086",
        "0x4227",
        """inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "iwx",
        "hbsd/src/sys/dev/iwx/if_iwx.c",
        "Network",
        "0x8086",
        "0x2526",
        """inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "otus",
        "hbsd/src/sys/dev/otus/if_otus.c",
        "Network",
        "0x0cf3",
        "0x1002",
        """inline constexpr std::uint32_t kRegMacCtl = 0x0000;

[[nodiscard]] inline Status validate_band(unsigned band) noexcept {
    return band < 3 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "sge",
        "hbsd/src/sys/dev/sge/if_sge.c",
        "Network",
        "0x10ec",
        "0x8168",
        """inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "enetc",
        "hbsd/src/sys/dev/enetc/if_enetc.c",
        "Network",
        "0x1131",
        "0x0000",
        """inline constexpr std::uint32_t kRegMode = 0x0000;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "dwc",
        "hbsd/src/sys/dev/dwc/if_dwc.c",
        "Network",
        "0x0000",
        "0x0000",
        """inline constexpr std::uint32_t kRegMacCfg = 0x0000;

[[nodiscard]] inline Status validate_mii(unsigned reg) noexcept {
    return reg <= 31 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "mvneta",
        "hbsd/src/sys/dev/neta/if_mvneta.c",
        "Network",
        "0x0000",
        "0x0000",
        """inline constexpr std::uint32_t kRegPortCfg = 0x0000;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 3 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "mxge",
        "hbsd/src/sys/dev/mxge/if_mxge.c",
        "Network",
        "0x15ad",
        "0x0001",
        """inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_slice(unsigned slice) noexcept {
    return slice < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "bwi",
        "hbsd/src/sys/dev/bwi/if_bwi.c",
        "Network",
        "0x14e4",
        "0x4315",
        """inline constexpr std::uint32_t kRegMacCtl = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 14 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "cas",
        "hbsd/src/sys/dev/cas/if_cas.c",
        "Network",
        "0x108e",
        "0x0021",
        """inline constexpr std::uint32_t kRegCfg = 0x0000;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 4 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "mge",
        "hbsd/src/sys/dev/mge/if_mge.c",
        "Network",
        "0x111d",
        "0x0001",
        """inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "wg",
        "hbsd/src/sys/dev/wg/if_wg.c",
        "Network",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kMaxPeers = 256;

[[nodiscard]] inline Status validate_peer(unsigned idx) noexcept {
    return idx < kMaxPeers ? Status::Ok : Status::Invalid;
}""",
    ),
]


def assert_keyword_safe(name: str) -> None:
    for seg in name.replace(".", "_").split("_"):
        if seg in KEYWORD_BAN:
            raise ValueError(f"keyword-banned segment: {seg} in {name}")


def ns_from_stem(stem: str) -> str:
    return stem.replace(".", "_")


def default_kernel_body(stem: str) -> str:
    return f"""enum class Op : unsigned char {{
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
    if (validate_op(op) != Status::Ok || !ctx.active) {{
        return Status::Invalid;
    }}
    ++ctx.count;
    return Status::Ok;
}}"""


def render_kernel_cppm(area: str, stem: str, provenance: str) -> str:
    assert_keyword_safe(stem)
    ns = ns_from_stem(stem)
    body = default_kernel_body(stem)
    return f"""module;
#include <cstdint>

export module pbsd.kernel.{stem};

import pbsd.core;

/// PROVENANCE: {provenance} — {stem.replace('_', ' ')} scaffold.
export namespace pbsd::kernel::{ns} {{

{body}

}} // namespace pbsd::kernel::{ns}
"""


def render_uda_cppm(stem: str, upstream: str, dev_class: str, vendor: str, device: str, body: str) -> str:
    assert_keyword_safe(stem)
    title = stem.replace("_", " ").title().replace(" ", "")
    return f"""module;
#include <cstdint>

export module pbsd.uda.{stem};

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: {upstream}
export namespace pbsd::uda::{stem} {{

{body}

inline constexpr RegInsn k{title}Init[] = {{
    {{RegOp::Done, 0, 0, 0, 0}},
}};

[[nodiscard]] inline constexpr Descriptor {stem}_generic() noexcept {{
    return Descriptor{{
        .name = "{stem}",
        .provenance = "{upstream}",
        .device_class = DeviceClass::{dev_class},
        .vendor_id = {vendor},
        .device_id = {device},
        .init_sequence = k{title}Init,
        .reset_sequence = k{title}Init,
    }};
}}

}} // namespace pbsd::uda::{stem}
"""


def kernel_target(stem: str) -> str:
    return f"pbsd_kernel_{stem.replace('.', '_')}"


def uda_target(stem: str) -> str:
    return f"pbsd_uda_{stem.replace('.', '_')}"


def write_if_missing(path: Path, body: str) -> bool:
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8", newline="\n")
    CREATED.append(str(path.relative_to(PBSD)).replace("\\", "/"))
    return True


def ensure_kernel(area: str, stem: str, provenance: str, parent: str | None) -> None:
    rel = f"kernel/{area}/pbsd.kernel.{stem}.cppm"
    cppm = PBSD / rel
    c_path = PBSD / f"kernel/{area}/pbsd.kernel.{stem}.c"
    if write_if_missing(cppm, render_kernel_cppm(area, stem, provenance)):
        deps = "pbsd_core" if not parent else f"pbsd_core {parent}"
        KERNEL_TARGETS.append((kernel_target(stem), rel, deps))
    if not c_path.exists():
        c_path.write_text(
            f"/* Reference logic from {provenance} (dual-world). */\n\n/* stub */\n",
            encoding="utf-8",
            newline="\n",
        )
        rel_c = str(c_path.relative_to(PBSD)).replace("\\", "/")
        if rel_c not in CREATED:
            CREATED.append(rel_c)


def ensure_uda(stem: str, upstream: str, dev_class: str, vendor: str, device: str, body: str) -> None:
    rel = f"uda/descriptors/{stem}.cppm"
    cppm = PBSD / rel
    c_path = PBSD / f"uda/descriptors/{stem}.c"
    if write_if_missing(cppm, render_uda_cppm(stem, upstream, dev_class, vendor, device, body)):
        UDA_TARGETS.append((uda_target(stem), rel, "pbsd_uda_schema pbsd_uda_interp"))
    if not c_path.exists():
        c_path.write_text(
            f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for pbsd.uda.{stem} */\n",
            encoding="utf-8",
            newline="\n",
        )
        rel_c = str(c_path.relative_to(PBSD)).replace("\\", "/")
        if rel_c not in CREATED:
            CREATED.append(rel_c)


def patch_cmake_kernel() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    anchor = "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)"
    blocks: list[str] = []
    for target, rel, deps in KERNEL_TARGETS:
        if f"TARGET {target}" in cmake:
            continue
        blocks.append(
            f"if(NOT TARGET {target})\n"
            f"add_library({target})\n"
            f"target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES\n"
            f"    {rel})\n"
            f"target_link_libraries({target} PUBLIC {deps})\n"
            f"target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})\n"
            f"endif()\n"
        )
    if blocks and anchor in cmake:
        cmake = cmake.replace(anchor, "\n".join(blocks) + "\n" + anchor, 1)
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return len(blocks)


def patch_cmake_uda() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    anchor = "if(NOT TARGET pbsd_uda)\nadd_library(pbsd_uda INTERFACE)"
    blocks: list[str] = []
    for target, rel, deps in UDA_TARGETS:
        if f"TARGET {target}" in cmake:
            continue
        blocks.append(
            f"if(NOT TARGET {target})\n"
            f"add_library({target})\n"
            f"target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES {rel})\n"
            f"target_link_libraries({target} PUBLIC {deps})\n"
            f"target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})\n"
            f"endif()\n"
        )
    if blocks and anchor in cmake:
        cmake = cmake.replace(anchor, "\n".join(blocks) + "\n" + anchor, 1)
        CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return len(blocks)


def patch_kernel_iface() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    tail = "    pbsd_kernel_madvise)"
    if tail not in cmake:
        return 0
    added = 0
    extras: list[str] = []
    for target, _, _ in KERNEL_TARGETS:
        if target not in cmake:
            extras.append(f"    {target}")
            added += 1
    if not extras:
        return 0
    cmake = cmake.replace(tail, "\n".join(extras) + "\n" + tail, 1)
    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def patch_uda_iface() -> int:
    cmake = CMAKE.read_text(encoding="utf-8")
    tail = "    pbsd_handles)"
    if tail not in cmake:
        return 0
    added = 0
    extras: list[str] = []
    for target, _, _ in UDA_TARGETS:
        if target not in cmake:
            extras.append(target)
            added += 1
    if not extras:
        return 0
    cmake = cmake.replace(tail, " ".join(extras) + " " + tail, 1)
    CMAKE.write_text(cmake, encoding="utf-8", newline="\n")
    return added


def main() -> None:
    for area, stem, prov, parent in KERNEL_MODULES:
        ensure_kernel(area, stem, prov, parent)
    for stem, upstream, dev_class, vendor, device, body in UDA_MODULES:
        ensure_uda(stem, upstream, dev_class, vendor, device, body)

    kcmake = patch_cmake_kernel()
    ucmake = patch_cmake_uda()
    klink = patch_kernel_iface()
    ulink = patch_uda_iface()

    cppm_created = [p for p in CREATED if p.endswith(".cppm")]
    print(f"created {len(cppm_created)} .cppm modules (+ {len(CREATED) - len(cppm_created)} .c stubs)")
    print(f"cmake kernel targets: +{kcmake}, uda targets: +{ucmake}")
    print(f"pbsd_kernel links: +{klink}, pbsd_uda links: +{ulink}")
    for p in sorted(cppm_created):
        print(f"  {p}")


if __name__ == "__main__":
    main()
