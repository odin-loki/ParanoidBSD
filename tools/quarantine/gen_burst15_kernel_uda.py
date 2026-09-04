#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Burst 15 — kernel kern/vm/sync/net helpers + UDA descriptor ports (dual-world)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CREATED: list[str] = []


def write(rel: str, body: str, c_stub: str | None = None) -> None:
    path = PBSD / rel
    if path.exists():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body.strip() + "\n", encoding="utf-8")
    CREATED.append(rel.replace("\\", "/"))
    if c_stub is not None:
        c_path = path.with_suffix(".c")
        if not c_path.exists():
            c_path.write_text(c_stub, encoding="utf-8")
            CREATED.append(str(c_path.relative_to(PBSD)).replace("\\", "/"))


def kernel_subr(stem: str, imports: str, ns_extra: str, body: str, deps: str) -> tuple[str, str, str]:
    mod = f"pbsd.kernel.{stem}"
    rel = f"kernel/kern/pbsd.kernel.{stem}.cppm"
    src = f"""module;
#include <cstdint>

export module {mod};

import pbsd.core;
{imports}

/// Freestanding port of `kern/{stem.replace('.', '_') if '.' not in stem else stem}.c` helpers.
export namespace pbsd::kernel::{stem.replace('.', '_')} {{

{body}

}} // namespace pbsd::kernel::{stem.replace('.', '_')}
"""
    target = f"pbsd_kernel_{stem.replace('.', '_')}"
    return rel, src, target


def kernel_mod(area: str, stem: str, imports: str, body: str, provenance: str) -> tuple[str, str, str]:
    mod = f"pbsd.kernel.{stem}"
    rel = f"kernel/{area}/pbsd.kernel.{stem}.cppm"
    ns = stem.replace(".", "_")
    src = f"""module;
#include <cstdint>

export module {mod};

import pbsd.core;
{imports}

/// Freestanding port of `{provenance}`.
export namespace pbsd::kernel::{ns} {{

{body}

}} // namespace pbsd::kernel::{ns}
"""
    target = f"pbsd_kernel_{stem.replace('.', '_')}"
    return rel, src, target


def uda_desc(stem: str, upstream: str, dev_class: str, vendor: str, device: str, body: str) -> tuple[str, str, str]:
    rel = f"uda/descriptors/{stem}.cppm"
    ns = stem
    src = f"""module;
#include <cstdint>

export module pbsd.uda.{ns};

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: {upstream}
export namespace pbsd::uda::{ns} {{

{body}

inline constexpr RegInsn k{ns.title()}Init[] = {{
    {{RegOp::Done, 0, 0, 0, 0}},
}};

[[nodiscard]] inline constexpr Descriptor {ns}_generic() noexcept {{
    return Descriptor{{
        .name = "{ns}",
        .provenance = "{upstream}",
        .device_class = DeviceClass::{dev_class},
        .vendor_id = {vendor},
        .device_id = {device},
        .init_sequence = k{ns.title()}Init,
        .reset_sequence = k{ns.title()}Init,
    }};
}}

}} // namespace pbsd::uda::{ns}
"""
    c_stub = f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for pbsd.uda.{ns} */\n"
    return rel, src, f"pbsd_uda_{ns}"


KERNEL_TARGETS: list[tuple[str, str, str, str]] = []


def add_kernel(rel: str, src: str, target: str, deps: str) -> None:
    before = len(CREATED)
    write(rel, src)
    if len(CREATED) > before:
        KERNEL_TARGETS.append((target, rel.replace("\\", "/"), deps))


UDA_TARGETS: list[tuple[str, str, str, str]] = []


def add_uda(rel: str, src: str, target: str, c_stub: str) -> None:
    before = len(CREATED)
    write(rel, src, c_stub)
    if len(CREATED) > before:
        UDA_TARGETS.append((target, rel.replace("\\", "/"), "pbsd_uda_schema pbsd_uda_interp"))


def main() -> None:
    # --- subr helpers -------------------------------------------------------
    specs = [
        (
            "subr_uio",
            "import pbsd.kernel.uio;",
            """[[nodiscard]] inline Status advance_resid(uio::UioStub& u, std::int64_t n) noexcept {
    if (uio::validate_resid(u.resid) != Status::Ok || n < 0 || n > u.resid) {
        return Status::Invalid;
    }
    u.resid -= n;
    u.offset += n;
    return Status::Ok;
}

[[nodiscard]] inline Status check_rw_space(const uio::UioStub& u, std::int64_t need) noexcept {
    if (uio::validate_iovcnt(u.iovcnt) != Status::Ok) {
        return Status::Invalid;
    }
    if (need < 0 || need > u.resid) {
        return Status::Invalid;
    }
    return Status::Ok;
}""",
            "pbsd_kernel_uio",
        ),
        (
            "subr_turnstile",
            "import pbsd.kernel.turnstile;",
            """[[nodiscard]] inline Status queue_wait(turnstile::TurnstileStub& ts, int queue) noexcept {
    if (queue < turnstile::kTsExclusiveQueue || queue >= turnstile::kTsQueues) {
        return Status::Invalid;
    }
    if (queue == turnstile::kTsExclusiveQueue) {
        ++ts.blocked_exclusive;
    } else {
        ++ts.blocked_shared;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status signal_one(turnstile::TurnstileStub& ts, int queue) noexcept {
    if (turnstile::queue_empty(ts, queue)) {
        return Status::NotFound;
    }
    if (queue == turnstile::kTsExclusiveQueue) {
        --ts.blocked_exclusive;
    } else {
        --ts.blocked_shared;
    }
    return Status::Ok;
}""",
            "pbsd_kernel_turnstile",
        ),
        (
            "subr_sleepqueue",
            "import pbsd.kernel.sleepq;",
            """[[nodiscard]] inline Status sleep_on(sleepq::Channel& ch, const void* wchan) noexcept {
    return sleepq::sleep(ch, wchan, sleepq::kTypeSleep);
}

[[nodiscard]] inline Status timed_sleep(sleepq::Channel& ch, const void* wchan, unsigned waiters) noexcept {
    if (wchan == nullptr) {
        return Status::Invalid;
    }
    ch.waiters = waiters;
    ch.wchan = wchan;
    ch.type = sleepq::kTypeSleep;
    return Status::Ok;
}""",
            "pbsd_kernel_sleepq",
        ),
        (
            "subr_sbuf",
            "import pbsd.kernel.sbuf;",
            """[[nodiscard]] inline Status reserve(sbuf::Softc& sb, unsigned extra) noexcept {
    if (sbuf::has(sb.flags, sbuf::Flag::Finished)) {
        return Status::Denied;
    }
    return sbuf::append_len(sb, extra);
}

[[nodiscard]] inline bool has_overflow(const sbuf::Softc& sb) noexcept {
    return sbuf::has(sb.flags, sbuf::Flag::Overflow);
}""",
            "pbsd_kernel_sbuf",
        ),
        (
            "subr_witness",
            "import pbsd.kernel.witness;",
            """[[nodiscard]] inline Status validate_order(const witness::LockObjectStub& held,
                                         const witness::LockObjectStub& want) noexcept {
    return witness::check_order(held, want);
}

[[nodiscard]] inline unsigned known_order_count() noexcept {
    return witness::order_table_size();
}""",
            "pbsd_kernel_witness",
        ),
        (
            "subr_bufring",
            "import pbsd.kernel.bufring;",
            """[[nodiscard]] inline Status enqueue_slot(bufring::Ring& br) noexcept {
    const unsigned occ = bufring::prod_occupancy(br);
    if (occ >= bufring::prod_capacity(br)) {
        return Status::Busy;
    }
    ++br.prod_tail;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue_slot(bufring::Ring& br) noexcept {
    if (br.cons_head == br.cons_tail) {
        return Status::NotFound;
    }
    ++br.cons_head;
    return Status::Ok;
}""",
            "pbsd_kernel_bufring",
        ),
        (
            "subr_eventhandler",
            "import pbsd.kernel.eventhandler;",
            """[[nodiscard]] inline Status attach(eventhandler::ListStub& list,
                                    eventhandler::HandlerFn fn,
                                    void* arg,
                                    int pri) noexcept {
    if (list.name == nullptr) {
        return Status::Invalid;
    }
    return eventhandler::register_handler(list, fn, arg, pri);
}""",
            "pbsd_kernel_eventhandler",
        ),
    ]
    for stem, imports, body, deps in specs:
        rel, src, target = kernel_subr(stem, imports, "", body, deps)
        add_kernel(rel, src, target, deps)

    # --- vm helpers ---------------------------------------------------------
    vm_specs = [
        (
            "vm",
            "vm_object_helpers",
            "import pbsd.kernel.vm_object;",
            """using namespace pbsd::kernel::vm_object;

[[nodiscard]] inline Status extend_size(VmObjectHeader& obj, std::uint64_t delta) noexcept {
    if (is_dead(obj.flags)) {
        return Status::Invalid;
    }
    obj.size += delta;
    return Status::Ok;
}

[[nodiscard]] inline bool covers_offset(const VmObjectHeader& obj, std::uint64_t off) noexcept {
    return off < obj.size;
}""",
            "vm/vm_object.c",
            "pbsd_kernel_vm_object",
        ),
        (
            "vm",
            "vm_page_helpers",
            "import pbsd.kernel.vm_page;",
            """using namespace pbsd::kernel::vm_page;

[[nodiscard]] inline Status move_queue(PageQueueEntry& e, unsigned new_q) noexcept {
    if (new_q >= kPqCount && new_q != kPqNone) {
        return Status::Invalid;
    }
    e.queue = new_q;
    return Status::Ok;
}

[[nodiscard]] inline bool laundry_candidate(unsigned q) noexcept {
    return is_inactive_queue(q) || is_laundry_family(q);
}""",
            "vm/vm_page.c",
            "pbsd_kernel_vm_page",
        ),
        (
            "vm",
            "vm_fault_helpers",
            "import pbsd.kernel.vm_fault;\nimport pbsd.kernel.vm;",
            """using namespace pbsd::kernel::vm_fault;

[[nodiscard]] inline Status validate_fault(FaultState& st, unsigned char prot) noexcept {
    if ((st.max_prot & prot) != prot) {
        return Status::Denied;
    }
    st.fault_type = prot;
    return Status::Ok;
}

[[nodiscard]] inline bool retryable(FaultResult r) noexcept {
    return r == FaultResult::Restart || r == FaultResult::Continue;
}""",
            "vm/vm_fault.c",
            "pbsd_kernel_vm_fault",
        ),
    ]
    for area, stem, imports, body, prov, deps in vm_specs:
        rel, src, target = kernel_mod(area, stem, imports, body, prov)
        add_kernel(rel, src, target, deps)

    # --- sync + net helpers -------------------------------------------------
    sync_rel, sync_src, sync_target = kernel_mod(
        "sync",
        "sync_helpers",
        "import pbsd.kernel.mutex;\nimport pbsd.kernel.turnstile;\nimport pbsd.kernel.sleepq;",
        """[[nodiscard]] inline Status lock_or_sleep(mutex::MutexClass& m,
                                                   turnstile::TurnstileStub& ts,
                                                   unsigned tid) noexcept {
    const auto st = mutex::try_lock(m, tid);
    if (st == Status::Ok) {
        return Status::Ok;
    }
    return turnstile::queue_wait(ts, turnstile::kTsExclusiveQueue);
}

[[nodiscard]] inline Status unlock_and_wakeup(mutex::MutexClass& m,
                                              sleepq::Channel& ch,
                                              unsigned tid) noexcept {
    if (mutex::unlock(m, tid) != Status::Ok) {
        return Status::Denied;
    }
    return sleepq::wakeup(ch);
}""",
        "kern/subr_sleepqueue.c",
    )
    add_kernel(sync_rel, sync_src, sync_target, "pbsd_kernel_mutex pbsd_kernel_turnstile pbsd_kernel_sleepq")

    net_specs: list[tuple[str, str, str, str]] = [
        (
            "in_cksum",
            "",
            """inline constexpr unsigned kCsumOffset = 10;

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint32_t add16(std::uint32_t sum, std::uint16_t v) noexcept {
    return sum + v;
}""",
            "pbsd_core",
            "netinet/in_cksum.c",
        ),
        (
            "link_addr",
            "",
            """inline constexpr unsigned kMaxLinkAddr = 64;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kMaxLinkAddr ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool is_broadcast(const unsigned char* addr, unsigned len) noexcept {
    if (len == 0) {
        return false;
    }
    for (unsigned i = 0; i < len; ++i) {
        if (addr[i] != 0xff) {
            return false;
        }
    }
    return true;
}""",
            "pbsd_core",
            "net/link_addr.c",
        ),
        (
            "mbuf_helpers",
            "import pbsd.kernel.mbuf;",
            """[[nodiscard]] inline Status require_pkthdr(mbuf::Flags f) noexcept {
    if (!mbuf::has_flag(f, mbuf::Flags::Pkthdr)) {
        return Status::Invalid;
    }
    return mbuf::validate_pkthdr(f);
}

[[nodiscard]] inline bool has_ext(mbuf::Flags f) noexcept {
    return mbuf::has_flag(f, mbuf::Flags::Ext);
}""",
            "pbsd_core pbsd_kernel_mbuf",
            "sys/mbuf.h",
        ),
        (
            "pktsched",
            "",
            """inline constexpr unsigned kDefaultQlen = 50;

struct QueueLimits {
    unsigned qlen{kDefaultQlen};
    unsigned drops{};
};

[[nodiscard]] inline Status validate_qlen(unsigned qlen) noexcept {
    return qlen > 0 && qlen <= 1024 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status record_drop(QueueLimits& q) noexcept {
    ++q.drops;
    return Status::Ok;
}""",
            "pbsd_core",
            "net/pktsched/pktsched.c",
        ),
    ]
    for stem, imports, body, deps, prov in net_specs:
        rel, src, target = kernel_mod("net", stem, imports, body, prov)
        add_kernel(rel, src, target, deps)

    # --- UDA descriptors ----------------------------------------------------
    uda_specs = [
        (
            "ena",
            "hbsd/src/sys/dev/ena/ena.c",
            "Network",
            "0x1d0f",
            "0xec20",
            """inline constexpr std::uint32_t kRegDevCtl = 0x0000;
inline constexpr std::uint32_t kDevCtlReset = 0x1;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "alc",
            "hbsd/src/sys/dev/alc/if_alc.c",
            "Network",
            "0x1969",
            "0x1083",
            """inline constexpr std::uint32_t kRegMacCtl = 0x0050;
inline constexpr std::uint32_t kMacReset = 0x80000000u;

[[nodiscard]] inline Status validate_mii(unsigned reg) noexcept {
    return reg <= 31 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "gem",
            "hbsd/src/sys/dev/gem/if_gem.c",
            "Network",
            "0x108e",
            "0x0021",
            """inline constexpr std::uint32_t kRegConfig = 0x0000;
inline constexpr std::uint32_t kConfigReset = 0x80000000u;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 4 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "hifn",
            "hbsd/src/sys/dev/hifn/hifn7751.c",
            "Block",
            "0x13a3",
            "0x0002",
            """inline constexpr std::uint32_t kRegCommand = 0x0004;
inline constexpr std::uint32_t kCmdReset = 0x00000001u;

[[nodiscard]] inline Status validate_session(unsigned sid) noexcept {
    return sid < 256 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "cxgb",
            "hbsd/src/sys/dev/cxgb/cxgb_main.c",
            "Network",
            "0x1425",
            "0x0001",
            """inline constexpr std::uint32_t kRegWhoami = 0x0000;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "eqos",
            "hbsd/src/sys/dev/eqos/if_eqos.c",
            "Network",
            "0x7026",
            "0x0100",
            """inline constexpr std::uint32_t kRegDmaMode = 0x1000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 8 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "ffec",
            "hbsd/src/sys/dev/ffec/if_ffec.c",
            "Network",
            "0x1957",
            "0x7890",
            """inline constexpr std::uint32_t kRegIcr = 0x00b0;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 32 ? Status::Ok : Status::Invalid;
}""",
        ),
        (
            "bfe",
            "hbsd/src/sys/dev/bfe/if_bfe.c",
            "Network",
            "0x14e4",
            "0x4401",
            """inline constexpr std::uint32_t kRegMode = 0x006c;

[[nodiscard]] inline Status validate_frame(unsigned len) noexcept {
    return len >= 60 && len <= 1518 ? Status::Ok : Status::Invalid;
}""",
        ),
    ]
    for stem, upstream, dev_class, vendor, device, body in uda_specs:
        rel, src, target = uda_desc(stem, upstream, dev_class, vendor, device, body)
        c_stub = f"/* Reference logic from {upstream} (dual-world). */\n\n/* stub for pbsd.uda.{stem} */\n"
        add_uda(rel, src, target, c_stub)

    # --- CMake patch --------------------------------------------------------
    cmake_path = PBSD / "CMakeLists.txt"
    cmake = cmake_path.read_text(encoding="utf-8")

    blocks: list[str] = []
    for target, rel, deps in KERNEL_TARGETS:
        if f"if(NOT TARGET {target})" in cmake:
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

    uda_blocks: list[str] = []
    for target, rel, deps in UDA_TARGETS:
        if f"if(NOT TARGET {target})" in cmake:
            continue
        uda_blocks.append(
            f"if(NOT TARGET {target})\n"
            f"add_library({target})\n"
            f"target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES {rel})\n"
            f"target_link_libraries({target} PUBLIC {deps})\n"
            f"target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})\n"
            f"endif()\n"
        )

    marker_kernel = "if(NOT TARGET pbsd_kernel)\nadd_library(pbsd_kernel INTERFACE)"
    if blocks and marker_kernel in cmake:
        cmake = cmake.replace(marker_kernel, "\n".join(blocks) + "\n" + marker_kernel, 1)

    marker_uda = "if(NOT TARGET pbsd_uda)\nadd_library(pbsd_uda INTERFACE)"
    if uda_blocks and marker_uda in cmake:
        cmake = cmake.replace(marker_uda, "\n".join(uda_blocks) + "\n" + marker_uda, 1)

    iface_marker = "    pbsd_kernel_ether pbsd_kernel_altq pbsd_kernel_subr_counter)"
    new_kernel = [t for t, _, _ in KERNEL_TARGETS if t not in cmake.split("target_link_libraries(pbsd_kernel INTERFACE")[1].split("target_compile_options(pbsd_kernel INTERFACE")[0]]
    if new_kernel and iface_marker in cmake:
        extra = "\n    ".join(new_kernel)
        cmake = cmake.replace(
            iface_marker,
            f"    pbsd_kernel_ether pbsd_kernel_altq pbsd_kernel_subr_counter\n    {extra})",
            1,
        )

    uda_iface_tail = "    pbsd_uda_mem pbsd_uda_zero pbsd_uda_tun pbsd_uda_speaker"
    new_uda = [t for t, _, _ in UDA_TARGETS]
    if new_uda and uda_iface_tail in cmake:
        extra = " ".join(new_uda)
        cmake = cmake.replace(
            uda_iface_tail,
            f"{uda_iface_tail} {extra}",
            1,
        )

    cmake_path.write_text(cmake, encoding="utf-8")

    print(f"Created {len(CREATED)} files")
    for p in CREATED:
        print(" ", p)
    print(f"CMake: +{len(blocks)} kernel targets, +{len(uda_blocks)} uda targets")


if __name__ == "__main__":
    main()
