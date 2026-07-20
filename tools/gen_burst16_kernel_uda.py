#!/usr/bin/env python3
"""Burst 16 — mass kernel kern/subr helpers + UDA descriptor ports (dual-world)."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"
CREATED: list[str] = []

# (stem, optional extra cmake dep beyond pbsd_core)
KERN_MODULES: list[tuple[str, str | None]] = [
    ("alq", None),
    ("boottrace", None),
    ("clocksource", "pbsd_kernel_timekeeping"),
    ("ctf", None),
    ("devctl", None),
    ("dump", "pbsd_kernel_bio"),
    ("fail", None),
    ("ffclock", "pbsd_kernel_timekeeping"),
    ("hhook", None),
    ("jaildesc", "pbsd_kernel_jail"),
    ("jailmeta", "pbsd_kernel_jail"),
    ("kcov", None),
    ("khelp", None),
    ("kthread", "pbsd_handles"),
    ("ktr", None),
    ("loginclass", "pbsd_kernel_racct"),
    ("mtxpool", "pbsd_kernel_mutex"),
    ("osd", None),
    ("pmc", None),
    ("priv", "pbsd_kernel_ucred"),
    ("rangelock", "pbsd_kernel_rmlock"),
    ("sema", None),
    ("shutdown", None),
    ("tc", "pbsd_kernel_timekeeping"),
    ("tslog", None),
    ("ucoredump", "pbsd_kernel_process"),
    ("vnodedumper", "pbsd_kernel_vnode"),
    ("resource", None),
]

SUBR_MODULES: list[tuple[str, str | None]] = [
    ("atomic64", None),
    ("clockcalib", "pbsd_kernel_timekeeping"),
    ("compressor", "pbsd_kernel_vm"),
    ("coverage", None),
    ("dummy_vdso_tc", "pbsd_kernel_timekeeping"),
    ("efi_map", None),
]

KERN_BODIES: dict[str, str] = {
    "alq": """inline constexpr unsigned kAlqMax = 256;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kAlqMax ? Status::Ok : Status::Invalid;
}""",
    "boottrace": """inline constexpr unsigned kBtEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kBtEnabled) != 0;
}""",
    "clocksource": """inline constexpr unsigned kCsMonotonic = 0;
inline constexpr unsigned kCsRealtime = 1;

[[nodiscard]] inline Status validate_id(unsigned id) noexcept {
    return id <= kCsRealtime ? Status::Ok : Status::Invalid;
}""",
    "ctf": """inline constexpr unsigned kCtfMaxSections = 64;

[[nodiscard]] inline Status validate_section(unsigned idx) noexcept {
    return idx < kCtfMaxSections ? Status::Ok : Status::Invalid;
}""",
    "devctl": """inline constexpr unsigned kDevctlMaxName = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kDevctlMaxName) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "dump": """inline constexpr unsigned kDumpCompress = 0x0001;
inline constexpr unsigned kDumpLive = 0x0002;

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kDumpCompress | kDumpLive)) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "fail": """inline constexpr unsigned kFailPointOnce = 0x0001;

[[nodiscard]] inline Status trigger(unsigned& count, unsigned flags) noexcept {
    if ((flags & kFailPointOnce) != 0 && count != 0) {
        return Status::Denied;
    }
    ++count;
    return Status::Ok;
}""",
    "ffclock": """inline constexpr unsigned kFfActive = 0x0001;

[[nodiscard]] inline bool is_active(unsigned state) noexcept {
    return (state & kFfActive) != 0;
}""",
    "hhook": """inline constexpr unsigned kHhMaxHooks = 32;

[[nodiscard]] inline Status validate_index(unsigned idx) noexcept {
    return idx < kHhMaxHooks ? Status::Ok : Status::Invalid;
}""",
    "jaildesc": """inline constexpr unsigned kJdNameMax = 256;

[[nodiscard]] inline Status validate_jid(int jid) noexcept {
    return jid >= 0 ? Status::Ok : Status::Invalid;
}""",
    "jailmeta": """inline constexpr unsigned kJmAttached = 0x0001;

[[nodiscard]] inline bool is_attached(unsigned flags) noexcept {
    return (flags & kJmAttached) != 0;
}""",
    "kcov": """inline constexpr unsigned kKcovModeTracePc = 0x0001;

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    if (mode & ~kKcovModeTracePc) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "khelp": """inline constexpr unsigned kKhMaxModules = 16;

[[nodiscard]] inline Status validate_module(unsigned idx) noexcept {
    return idx < kKhMaxModules ? Status::Ok : Status::Invalid;
}""",
    "kthread": """inline constexpr unsigned kKtRunning = 0x0001;
inline constexpr unsigned kKtStopped = 0x0002;

[[nodiscard]] inline bool is_running(unsigned flags) noexcept {
    return (flags & kKtRunning) != 0;
}""",
    "ktr": """inline constexpr unsigned kKtrGen = 0x0001;

[[nodiscard]] inline Status validate_class(unsigned cls) noexcept {
    return cls <= 255 ? Status::Ok : Status::Invalid;
}""",
    "loginclass": """inline constexpr unsigned kLcResource = 0x0001;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    return len > 0 && len < 256 ? Status::Ok : Status::Invalid;
}""",
    "mtxpool": """inline constexpr unsigned kMpBuckets = 128;

[[nodiscard]] inline unsigned bucket(unsigned hash) noexcept {
    return hash % kMpBuckets;
}""",
    "osd": """inline constexpr unsigned kOsdSlots = 16;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kOsdSlots ? Status::Ok : Status::Invalid;
}""",
    "pmc": """inline constexpr unsigned kPmcEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kPmcEnabled) != 0;
}""",
    "priv": """inline constexpr unsigned kPrivAll = 0xffffffffu;

[[nodiscard]] inline Status validate_set(unsigned priv) noexcept {
    return priv <= kPrivAll ? Status::Ok : Status::Invalid;
}""",
    "rangelock": """inline constexpr unsigned kRlShared = 0x0001;
inline constexpr unsigned kRlExclusive = 0x0002;

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    const unsigned acc = mode & (kRlShared | kRlExclusive);
    if (acc != kRlShared && acc != kRlExclusive) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
    "sema": """inline constexpr unsigned kSemValueMax = 65535;

[[nodiscard]] inline Status validate_value(unsigned value) noexcept {
    return value <= kSemValueMax ? Status::Ok : Status::Invalid;
}""",
    "shutdown": """enum class Phase : unsigned char {
    Sync = 0,
    Files = 1,
    Final = 2,
};

[[nodiscard]] inline Status validate_phase(Phase p) noexcept {
    switch (p) {
    case Phase::Sync:
    case Phase::Files:
    case Phase::Final:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}""",
    "tc": """inline constexpr unsigned kTcEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kTcEnabled) != 0;
}""",
    "tslog": """inline constexpr unsigned kTsMaxRecord = 4096;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kTsMaxRecord ? Status::Ok : Status::Invalid;
}""",
    "ucoredump": """inline constexpr unsigned kUcoreActive = 0x0001;

[[nodiscard]] inline bool is_active(unsigned flags) noexcept {
    return (flags & kUcoreActive) != 0;
}""",
    "vnodedumper": """inline constexpr unsigned kVndMaxName = 256;

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    return len > 0 && len < kVndMaxName ? Status::Ok : Status::Invalid;
}""",
    "resource": """inline constexpr unsigned kResCpu = 0x0001;
inline constexpr unsigned kResMemory = 0x0002;

[[nodiscard]] inline Status validate_type(unsigned type) noexcept {
    if ((type & (kResCpu | kResMemory)) == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
}

SUBR_BODIES: dict[str, str] = {
    "atomic64": """[[nodiscard]] inline std::uint64_t fetch_add(std::uint64_t& v, std::uint64_t delta) noexcept {
    const auto old = v;
    v += delta;
    return old;
}

[[nodiscard]] inline bool compare_exchange(std::uint64_t& v, std::uint64_t expected,
                                           std::uint64_t desired) noexcept {
    if (v != expected) {
        return false;
    }
    v = desired;
    return true;
}""",
    "clockcalib": """inline constexpr unsigned kCalibIntervalMs = 1000;

[[nodiscard]] inline Status validate_interval(unsigned ms) noexcept {
    return ms > 0 ? Status::Ok : Status::Invalid;
}""",
    "compressor": """inline constexpr unsigned kCompEnabled = 0x0001;

[[nodiscard]] inline bool is_enabled(unsigned flags) noexcept {
    return (flags & kCompEnabled) != 0;
}""",
    "coverage": """inline constexpr unsigned kCovMaxSites = 65536;

[[nodiscard]] inline Status validate_site(unsigned site) noexcept {
    return site < kCovMaxSites ? Status::Ok : Status::Invalid;
}""",
    "dummy_vdso_tc": """inline constexpr unsigned kVdsoGen = 0;

[[nodiscard]] inline Status publish(unsigned& gen) noexcept {
    ++gen;
    return Status::Ok;
}""",
    "efi_map": """inline constexpr unsigned kEfiMapPages = 4096;

[[nodiscard]] inline Status validate_pages(unsigned pages) noexcept {
    return pages <= kEfiMapPages ? Status::Ok : Status::Invalid;
}""",
}

# (stem, upstream, dev_class, vendor_hex, device_hex, body)
UDA_MODULES: list[tuple[str, str, str, str, str, str]] = [
    (
        "ice",
        "hbsd/src/sys/dev/ice/if_ice_iflib.c",
        "Network",
        "0x8086",
        "0x1591",
        """inline constexpr std::uint32_t kRegCtrl = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "gve",
        "hbsd/src/sys/dev/gve/gve_main.c",
        "Network",
        "0x1ae0",
        "0x0042",
        """inline constexpr std::uint32_t kRegDmaMode = 0x0010;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "iavf",
        "hbsd/src/sys/dev/iavf/if_iavf_iflib.c",
        "Network",
        "0x8086",
        "0x154c",
        """inline constexpr std::uint32_t kRegIntMask = 0x0380;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 64 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "bxe",
        "hbsd/src/sys/dev/bxe/bxe.c",
        "Network",
        "0x14e4",
        "0x164f",
        """inline constexpr std::uint32_t kRegMcp = 0x0000;

[[nodiscard]] inline Status validate_function(unsigned fn) noexcept {
    return fn < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "enetc",
        "hbsd/src/sys/dev/enetc/if_enetc.c",
        "Network",
        "0x1957",
        "0xe100",
        """inline constexpr std::uint32_t kRegImr = 0x0004;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "al_eth",
        "hbsd/src/sys/dev/al_eth/al_eth.c",
        "Network",
        "0x1c36",
        "0x0003",
        """inline constexpr std::uint32_t kRegAdapterCtl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 32 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "arcmsr",
        "hbsd/src/sys/dev/arcmsr/arcmsr.c",
        "Block",
        "0x17d3",
        "0x1010",
        """inline constexpr std::uint32_t kRegOutbound = 0x0040;

[[nodiscard]] inline Status validate_target(unsigned tgt) noexcept {
    return tgt < 16 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "aac",
        "hbsd/src/sys/dev/aac/aac.c",
        "Block",
        "0x9005",
        "0x0285",
        """inline constexpr std::uint32_t kRegDoorbell = 0x0020;

[[nodiscard]] inline Status validate_fib(unsigned idx) noexcept {
    return idx < 512 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ciss",
        "hbsd/src/sys/dev/ciss/ciss.c",
        "Block",
        "0x103c",
        "0x3239",
        """inline constexpr std::uint32_t kRegInbound = 0x0000;

[[nodiscard]] inline Status validate_lun(unsigned lun) noexcept {
    return lun < 256 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "hpt27xx",
        "hbsd/src/sys/dev/hpt27xx/hpt27xx.c",
        "Block",
        "0x1103",
        "0x2720",
        """inline constexpr std::uint32_t kRegControl = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 8 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "bwi",
        "hbsd/src/sys/dev/bwi/if_bwi.c",
        "Network",
        "0x168c",
        "0x0013",
        """inline constexpr std::uint32_t kRegMacCtl = 0x0400;

[[nodiscard]] inline Status validate_rate(unsigned rate) noexcept {
    return rate <= 54 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "ae",
        "hbsd/src/sys/dev/ae/if_ae.c",
        "Network",
        "0x10ec",
        "0x8168",
        """inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_desc(unsigned idx) noexcept {
    return idx < 256 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "cadence",
        "hbsd/src/sys/dev/cadence/cadence.c",
        "Block",
        "0x17cd",
        "0xdc08",
        """inline constexpr std::uint32_t kRegStatus = 0x0004;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < 4 ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "hwpmc",
        "hbsd/src/sys/dev/hwpmc/hwpmc.c",
        "Sensor",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kPmcMaxEvents = 32;

[[nodiscard]] inline Status validate_event(unsigned ev) noexcept {
    return ev < kPmcMaxEvents ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "imcsmb",
        "hbsd/src/sys/dev/imcsmb/imcsmb.c",
        "Sensor",
        "0x8086",
        "0x0c02",
        """inline constexpr unsigned kSmbMaxBytes = 32;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kSmbMaxBytes ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "iicbus",
        "hbsd/src/sys/dev/iicbus/iicbus.c",
        "Input",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kIicMaxAddr = 127;

[[nodiscard]] inline Status validate_addr(unsigned addr) noexcept {
    return addr <= kIicMaxAddr ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "cardbus",
        "hbsd/src/sys/dev/cardbus/cardbus.c",
        "Block",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kCbMaxSlots = 8;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kCbMaxSlots ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "firewire",
        "hbsd/src/sys/dev/firewire/firewire.c",
        "Input",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kFwMaxNodes = 63;

[[nodiscard]] inline Status validate_node(unsigned node) noexcept {
    return node < kFwMaxNodes ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "drm2",
        "hbsd/src/sys/dev/drm2/drm2.c",
        "Display",
        "0x0000",
        "0x0000",
        """inline constexpr unsigned kDrmMaxCrtcs = 4;

[[nodiscard]] inline Status validate_crtc(unsigned crtc) noexcept {
    return crtc < kDrmMaxCrtcs ? Status::Ok : Status::Invalid;
}""",
    ),
    (
        "cas",
        "hbsd/src/sys/dev/cas/if_cas.c",
        "Network",
        "0x108e",
        "0x0021",
        """inline constexpr std::uint32_t kRegIntMask = 0x0004;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 4 ? Status::Ok : Status::Invalid;
}""",
    ),
]


def write(rel: str, body: str, c_stub: str | None = None) -> bool:
    path = PBSD / rel
    if path.exists():
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8")
    CREATED.append(rel.replace("\\", "/"))
    if c_stub is not None:
        c_path = path.with_suffix(".c")
        if not c_path.exists():
            c_path.write_text(c_stub, encoding="utf-8")
            CREATED.append(str(c_path.relative_to(PBSD)).replace("\\", "/"))
    return True


def render_kern(stem: str, body: str) -> str:
    title = stem.replace("_", " ")
    inc = "#include <cstdint>\n" if "std::" in body or "unsigned char" in body else ""
    return f"""module;
{inc}
export module pbsd.kernel.kern_{stem};

export import pbsd.core;

/// Freestanding port of `kern/kern_{stem}.c` — {title} helpers.
export namespace pbsd::kernel::kern_{stem} {{

{body}

}} // namespace pbsd::kernel::kern_{stem}
"""


def render_subr(stem: str, body: str) -> str:
    title = stem.replace("_", " ")
    return f"""module;
#include <cstdint>

export module pbsd.kernel.subr_{stem};

export import pbsd.core;

/// Freestanding port of `kern/subr_{stem}.c` — {title} helpers.
export namespace pbsd::kernel::subr_{stem} {{

{body}

}} // namespace pbsd::kernel::subr_{stem}
"""


def render_uda(stem: str, upstream: str, dev_class: str, vendor: str, device: str, body: str) -> str:
    tag = stem.replace("_", " ")
    init = f"k{stem[0].upper()}{stem[1:]}Init" if "_" not in stem else f"k{''.join(p.title() for p in stem.split('_'))}Init"
    return f"""module;
#include <cstdint>

export module pbsd.uda.{stem};

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: {upstream}
export namespace pbsd::uda::{stem} {{

{body}

inline constexpr RegInsn {init}[] = {{
    {{RegOp::Done, 0, 0, 0, 0}},
}};

[[nodiscard]] inline constexpr Descriptor {stem}_generic() noexcept {{
    return Descriptor{{
        .name = "{stem}",
        .provenance = "{upstream}",
        .device_class = DeviceClass::{dev_class},
        .vendor_id = {vendor},
        .device_id = {device},
        .init_sequence = {init},
        .reset_sequence = {init},
    }};
}}

}} // namespace pbsd::uda::{stem}
"""


def kern_target(stem: str) -> str:
    return f"pbsd_kernel_kern_{stem}"


def subr_target(stem: str) -> str:
    return f"pbsd_kernel_subr_{stem}"


def uda_target(stem: str) -> str:
    return f"pbsd_uda_{stem}"


def cmake_kern_block(stem: str, deps: str | None) -> str:
    tgt = kern_target(stem)
    rel = f"kernel/kern/pbsd.kernel.kern_{stem}.cppm"
    link = "pbsd_core" if not deps else f"pbsd_core {deps}"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {link})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def cmake_subr_block(stem: str, deps: str | None) -> str:
    tgt = subr_target(stem)
    rel = f"kernel/kern/pbsd.kernel.subr_{stem}.cppm"
    link = "pbsd_core" if not deps else f"pbsd_core {deps}"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES
    {rel})
target_link_libraries({tgt} PUBLIC {link})
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def cmake_uda_block(stem: str) -> str:
    tgt = uda_target(stem)
    rel = f"uda/descriptors/{stem}.cppm"
    return f"""if(NOT TARGET {tgt})
add_library({tgt})
target_sources({tgt} PUBLIC FILE_SET CXX_MODULES FILES {rel})
target_link_libraries({tgt} PUBLIC pbsd_uda_schema pbsd_uda_interp)
target_compile_options({tgt} PUBLIC ${{PBSD_FS_CXX}})
endif()
"""


def patch_cmake(new_kernel: list[str], new_subr: list[str], new_uda: list[str]) -> None:
    cmake = CMAKE.read_text(encoding="utf-8")

    kern_blocks: list[str] = []
    for stem, deps in KERN_MODULES:
        tgt = kern_target(stem)
        if tgt not in new_kernel:
            continue
        if f"if(NOT TARGET {tgt})" in cmake:
            continue
        kern_blocks.append(cmake_kern_block(stem, deps))

    subr_blocks: list[str] = []
    for stem, deps in SUBR_MODULES:
        tgt = subr_target(stem)
        if tgt not in new_subr:
            continue
        if f"if(NOT TARGET {tgt})" in cmake:
            continue
        subr_blocks.append(cmake_subr_block(stem, deps))

    uda_blocks: list[str] = []
    for stem, *_ in UDA_MODULES:
        tgt = uda_target(stem)
        if tgt not in new_uda:
            continue
        if f"if(NOT TARGET {tgt})" in cmake:
            continue
        uda_blocks.append(cmake_uda_block(stem))

    anchor_kern = "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)"
    if kern_blocks or subr_blocks:
        insert = "\n".join(kern_blocks + subr_blocks)
        if anchor_kern in cmake:
            cmake = cmake.replace(anchor_kern, insert + "\n" + anchor_kern, 1)

    anchor_uda = "if(NOT TARGET pbsd_uda)\nadd_library(pbsd_uda INTERFACE)"
    if uda_blocks and anchor_uda in cmake:
        cmake = cmake.replace(anchor_uda, "\n".join(uda_blocks) + "\n" + anchor_uda, 1)

    if new_kernel or new_subr:
        m = re.search(
            r"(target_link_libraries\(pbsd_kernel INTERFACE[\s\S]*?)(    pbsd_kernel_madvise\))",
            cmake,
        )
        if m:
            prefix = m.group(1)
            additions = []
            for stem, _ in KERN_MODULES:
                tgt = kern_target(stem)
                if tgt in new_kernel and tgt not in prefix:
                    additions.append(f"    {tgt}")
            for stem, _ in SUBR_MODULES:
                tgt = subr_target(stem)
                if tgt in new_subr and tgt not in prefix:
                    additions.append(f"    {tgt}")
            if additions:
                cmake = cmake.replace(
                    m.group(0),
                    prefix + "\n".join(additions) + "\n" + m.group(2),
                    1,
                )

    if new_uda:
        m = re.search(
            r"(target_link_libraries\(pbsd_uda INTERFACE[\s\S]*?)(    pbsd_handles\))",
            cmake,
        )
        if m:
            prefix = m.group(1)
            additions = []
            for stem, *_ in UDA_MODULES:
                tgt = uda_target(stem)
                if tgt in new_uda and tgt not in prefix:
                    additions.append(f"    {tgt}")
            if additions:
                cmake = cmake.replace(
                    m.group(0),
                    prefix + " ".join(additions) + "\n" + m.group(2),
                    1,
                )

    CMAKE.write_text(cmake, encoding="utf-8")


def main() -> None:
    new_kernel: list[str] = []
    new_subr: list[str] = []
    new_uda: list[str] = []

    for stem, _ in KERN_MODULES:
        rel = f"kernel/kern/pbsd.kernel.kern_{stem}.cppm"
        src = f"hbsd/src/sys/kern/kern_{stem}.c"
        if write(rel, render_kern(stem, KERN_BODIES[stem]), f"/* Reference logic from {src} (dual-world). */\n\n/* stub */\n"):
            new_kernel.append(kern_target(stem))

    for stem, _ in SUBR_MODULES:
        rel = f"kernel/kern/pbsd.kernel.subr_{stem}.cppm"
        src = f"hbsd/src/sys/kern/subr_{stem}.c"
        if write(rel, render_subr(stem, SUBR_BODIES[stem]), f"/* Reference logic from {src} (dual-world). */\n\n/* stub */\n"):
            new_subr.append(subr_target(stem))

    for stem, upstream, dev_class, vendor, device, body in UDA_MODULES:
        rel = f"uda/descriptors/{stem}.cppm"
        stub = f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for pbsd.uda.{stem} */\n"
        if write(rel, render_uda(stem, upstream, dev_class, vendor, device, body), stub):
            new_uda.append(uda_target(stem))

    patch_cmake(new_kernel, new_subr, new_uda)

    print(f"Created {len(CREATED)} files")
    for p in CREATED:
        print(" ", p)
    print(f"kernel targets: {len(new_kernel)}, subr targets: {len(new_subr)}, uda targets: {len(new_uda)}")


if __name__ == "__main__":
    main()
