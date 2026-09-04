#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Wave 4/5 kernel + UDA hand-port modules and patch pbsd/CMakeLists.txt."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

# (cmake_target, cppm_path_relative_to_pbsd, link_deps)
KERNEL_NEW = [
    ("pbsd_kernel_rmlock", "kernel/sync/pbsd.kernel.rmlock.cppm", ["pbsd_core"]),
    ("pbsd_kernel_bio", "kernel/io/pbsd.kernel.bio.cppm", ["pbsd_core"]),
    ("pbsd_kernel_disk", "kernel/io/pbsd.kernel.disk.cppm", ["pbsd_core"]),
    ("pbsd_kernel_buf", "kernel/io/pbsd.kernel.buf.cppm", ["pbsd_core"]),
    ("pbsd_kernel_eventhandler", "kernel/kern/pbsd.kernel.eventhandler.cppm", ["pbsd_core"]),
    ("pbsd_kernel_refcnt", "kernel/kern/pbsd.kernel.refcnt.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kobj", "kernel/kern/pbsd.kernel.kobj.cppm", ["pbsd_core"]),
    ("pbsd_kernel_pci", "kernel/kern/pbsd.kernel.pci.cppm", ["pbsd_core"]),
    ("pbsd_kernel_devstat", "kernel/kern/pbsd.kernel.devstat.cppm", ["pbsd_core"]),
]

KERNEL_ORPHAN = [
    ("pbsd_kernel_sx", "kernel/sync/pbsd.kernel.sx.cppm", ["pbsd_core"]),
    ("pbsd_kernel_rwlock", "kernel/sync/pbsd.kernel.rwlock.cppm", ["pbsd_core"]),
    ("pbsd_kernel_condvar", "kernel/sync/pbsd.kernel.condvar.cppm", ["pbsd_core"]),
    ("pbsd_kernel_taskqueue", "kernel/kern/pbsd.kernel.taskqueue.cppm", ["pbsd_core"]),
    ("pbsd_kernel_uio", "kernel/kern/pbsd.kernel.uio.cppm", ["pbsd_core"]),
    ("pbsd_kernel_lockf", "kernel/kern/pbsd.kernel.lockf.cppm", ["pbsd_core"]),
    ("pbsd_kernel_sleepq", "kernel/sleepq/pbsd.kernel.sleepq.cppm", ["pbsd_core"]),
]

UDA_NEW = [
    ("pbsd_uda_vr", "uda/descriptors/vr.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_sis", "uda/descriptors/sis.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_bce", "uda/descriptors/bce.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_nfe", "uda/descriptors/nfe.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_jme", "uda/descriptors/jme.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_vge", "uda/descriptors/vge.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_bwn", "uda/descriptors/bwn.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_rum", "uda/descriptors/rum.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_wpi", "uda/descriptors/wpi.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_mwl", "uda/descriptors/mwl.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_zyd", "uda/descriptors/zyd.cppm", ["pbsd_uda_schema"]),
]

UDA_ORPHAN = [
    ("pbsd_uda_fxp", "uda/descriptors/fxp.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_axe", "uda/descriptors/axe.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_ale", "uda/descriptors/ale.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_virtio_input", "uda/descriptors/virtio_input.cppm",
     ["pbsd_uda_virtio_common"]),
    ("pbsd_uda_vmx", "uda/descriptors/vmx.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
]

KERNEL_MODULES = {
    "kernel/sync/pbsd.kernel.rmlock.cppm": """module;
#include <cstdint>

export module pbsd.kernel.rmlock;

export import pbsd.core;

/// Wave 4/5 — rmlock init flags from sys/rmlock.h.
export namespace pbsd::kernel::rmlock {

inline constexpr unsigned kNowitness  = 0x00000001;
inline constexpr unsigned kRecurse    = 0x00000002;
inline constexpr unsigned kSleepable  = 0x00000004;
inline constexpr unsigned kNew        = 0x00000008;
inline constexpr unsigned kDupok      = 0x00000010;

struct RmlockStub {
    unsigned opts{};
    bool     write_held{};
    unsigned readers{};
};

[[nodiscard]] constexpr bool is_sleepable(unsigned opts) noexcept {
    return (opts & kSleepable) != 0;
}

[[nodiscard]] constexpr Status validate_opts(unsigned opts) noexcept {
    if ((opts & ~(kNowitness | kRecurse | kSleepable | kNew | kDupok)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::rmlock
""",
    "kernel/io/pbsd.kernel.bio.cppm": """module;
#include <cstdint>

export module pbsd.kernel.bio;

export import pbsd.core;

/// Wave 4/5 — GEOM bio commands/flags from sys/bio.h.
export namespace pbsd::kernel::bio {

inline constexpr unsigned char kRead    = 0x01;
inline constexpr unsigned char kWrite   = 0x02;
inline constexpr unsigned char kDelete  = 0x03;
inline constexpr unsigned char kGetattr = 0x04;
inline constexpr unsigned char kFlush   = 0x05;
inline constexpr unsigned char kZone    = 0x09;
inline constexpr unsigned char kSpeedup = 0x0a;

inline constexpr unsigned kDone           = 0x02;
inline constexpr unsigned kOnqueue        = 0x04;
inline constexpr unsigned kOrdered        = 0x08;
inline constexpr unsigned kUnmapped       = 0x10;
inline constexpr unsigned kTransientMap   = 0x20;
inline constexpr unsigned kVlist          = 0x40;
inline constexpr unsigned kSwap           = 0x200;
inline constexpr unsigned kError          = 0x1000;
inline constexpr unsigned kExterr         = 0x2000;
inline constexpr unsigned kSpeedupWrite   = 0x4000;
inline constexpr unsigned kSpeedupTrim    = 0x8000;

[[nodiscard]] constexpr bool is_read(unsigned char cmd) noexcept {
    return cmd == kRead;
}

[[nodiscard]] constexpr bool is_write(unsigned char cmd) noexcept {
    return cmd == kWrite || cmd == kDelete;
}

[[nodiscard]] constexpr Status validate_cmd(unsigned char cmd) noexcept {
    if (cmd < kRead || (cmd > kFlush && cmd < kZone) || cmd > kSpeedup) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::bio
""",
    "kernel/io/pbsd.kernel.disk.cppm": """module;

export module pbsd.kernel.disk;

export import pbsd.core;

/// Wave 4/5 — disk ioctls from sys/disk.h.
export namespace pbsd::kernel::disk {

inline constexpr unsigned kIdentSize = 256;

enum class Ioctl : unsigned {
    SectorSize   = 128,
    MediaSize    = 129,
    FwSectors    = 130,
    FwHeads      = 131,
    Flush        = 135,
    Delete       = 136,
    Ident        = 137,
    ProviderName = 138,
    StripeSize   = 139,
    StripeOffset = 140,
    PhyPath      = 141,
    Attr         = 142,
    KernelDump   = 146,
};

[[nodiscard]] constexpr Status validate_ident_len(unsigned len) noexcept {
    if (len == 0 || len > kIdentSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::disk
""",
    "kernel/io/pbsd.kernel.buf.cppm": """module;
#include <cstdint>

export module pbsd.kernel.buf;

export import pbsd.core;

/// Wave 4/5 — buffer flags from sys/buf.h.
export namespace pbsd::kernel::buf {

inline constexpr std::uint32_t kAge           = 0x00000001;
inline constexpr std::uint32_t kNeedCommit    = 0x00000002;
inline constexpr std::uint32_t kAsync         = 0x00000004;
inline constexpr std::uint32_t kDirect       = 0x00000008;
inline constexpr std::uint32_t kDeferred      = 0x00000010;
inline constexpr std::uint32_t kCache         = 0x00000020;
inline constexpr std::uint32_t kValidSuspWrt  = 0x00000040;
inline constexpr std::uint32_t kInval         = 0x00002000;
inline constexpr std::uint32_t kLocked        = 0x00004000;
inline constexpr std::uint32_t kBusy          = 0x00008000;
inline constexpr std::uint32_t kDone          = 0x00010000;
inline constexpr std::uint32_t kDelWri        = 0x00020000;
inline constexpr std::uint32_t kClusterOk     = 0x00200000;
inline constexpr std::uint32_t kPaging        = 0x00400000;
inline constexpr std::uint32_t kRelBuf        = 0x00800000;
inline constexpr std::uint32_t kVmio          = 0x01000000;

struct BufStub {
    std::uint32_t flags{};
    int           error{};
};

[[nodiscard]] constexpr bool is_async(std::uint32_t flags) noexcept {
    return (flags & kAsync) != 0;
}

[[nodiscard]] constexpr bool is_dirty(std::uint32_t flags) noexcept {
    return (flags & kDelWri) != 0;
}

[[nodiscard]] constexpr Status validate_flags(std::uint32_t flags) noexcept {
    if ((flags & kDone) != 0 && (flags & kBusy) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::buf
""",
    "kernel/kern/pbsd.kernel.eventhandler.cppm": """module;

export module pbsd.kernel.eventhandler;

export import pbsd.core;

/// Wave 4/5 — eventhandler priorities from sys/eventhandler.h.
export namespace pbsd::kernel::eventhandler {

inline constexpr int kPriFirst = 0;
inline constexpr int kPriAny     = 10000;
inline constexpr int kPriLast  = 20000;

enum class ShutdownPri : int {
    First   = kPriFirst,
    Default = kPriAny,
    Last    = kPriLast,
};

enum class DevEvent : unsigned char {
    DetachBegin,
    DetachComplete,
    DetachFailed,
};

[[nodiscard]] constexpr Status validate_priority(int pri) noexcept {
    if (pri < kPriFirst || pri > kPriLast) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::eventhandler
""",
    "kernel/kern/pbsd.kernel.refcnt.cppm": """module;
#include <cstdint>

export module pbsd.kernel.refcnt;

export import pbsd.core;

/// Wave 4/5 — reference count helpers from sys/refcount.h.
export namespace pbsd::kernel::refcnt {

struct Refcount {
    std::uint32_t count{};
};

[[nodiscard]] inline Status acquire(Refcount& r) noexcept {
    if (r.count == ~0u) {
        return Status::Invalid;
    }
    ++r.count;
    return Status::Ok;
}

[[nodiscard]] inline Status release(Refcount& r) noexcept {
    if (r.count == 0) {
        return Status::Invalid;
    }
    --r.count;
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_shared(const Refcount& r) noexcept {
    return r.count > 1;
}

} // namespace pbsd::kernel::refcnt
""",
    "kernel/kern/pbsd.kernel.kobj.cppm": """module;

export module pbsd.kernel.kobj;

export import pbsd.core;

/// Wave 4/5 — kobj method dispatch stubs from sys/kobj.h.
export namespace pbsd::kernel::kobj {

inline constexpr unsigned kMethodTableSize = 64;

struct KobjClass {
    const char* name{};
    unsigned    method_count{};
};

[[nodiscard]] constexpr Status validate_class(const KobjClass& c) noexcept {
    if (c.name == nullptr || c.method_count == 0 || c.method_count > kMethodTableSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kobj
""",
    "kernel/kern/pbsd.kernel.pci.cppm": """module;
#include <cstdint>

export module pbsd.kernel.pci;

export import pbsd.core;

/// Wave 4/5 — PCI config space offsets from sys/pciio.h / pci_regs.h.
export namespace pbsd::kernel::pci {

inline constexpr std::uint8_t kCfgVendorId  = 0x00;
inline constexpr std::uint8_t kCfgDeviceId  = 0x02;
inline constexpr std::uint8_t kCfgCommand   = 0x04;
inline constexpr std::uint8_t kCfgStatus    = 0x06;
inline constexpr std::uint8_t kCfgRevId     = 0x08;
inline constexpr std::uint8_t kCfgClass     = 0x09;
inline constexpr std::uint8_t kCfgBar0      = 0x10;
inline constexpr std::uint8_t kCfgBar1      = 0x14;
inline constexpr std::uint8_t kCfgSubVendor = 0x2c;
inline constexpr std::uint8_t kCfgSubDevice = 0x2e;

inline constexpr std::uint16_t kCmdIoEnable  = 0x0001;
inline constexpr std::uint16_t kCmdMemEnable = 0x0002;
inline constexpr std::uint16_t kCmdBusMaster = 0x0004;

[[nodiscard]] constexpr Status validate_bar_index(unsigned idx) noexcept {
    if (idx > 5) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::pci
""",
    "kernel/kern/pbsd.kernel.devstat.cppm": """module;

export module pbsd.kernel.devstat;

export import pbsd.core;

/// Wave 4/5 — devstat tags from sys/devstat.h.
export namespace pbsd::kernel::devstat {

inline constexpr unsigned kTypeDirect = 0;
inline constexpr unsigned kTypeSequential = 1;
inline constexpr unsigned kTypePass = 2;

enum class Priority : unsigned char {
    Other = 0,
    Batch = 1,
    Idle  = 2,
    Standby = 3,
    Interactive = 4,
};

[[nodiscard]] constexpr Status validate_type(unsigned t) noexcept {
    if (t > kTypePass) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::devstat
""",
}

UDA_MODULES = {
    "uda/descriptors/vr.cppm": """module;
#include <cstdint>

export module pbsd.uda.vr;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vr/if_vrreg.h
export namespace pbsd::uda::vr {

inline constexpr std::uint32_t kRegCr1 = 0x09;
inline constexpr std::uint8_t  kCr1Reset = 0x80;

inline constexpr std::uint16_t kPciVendorVia = 0x1106;
inline constexpr std::uint16_t kPciDevRhine = 0x3065;

inline constexpr RegInsn kVrInit[] = {
    {RegOp::Write8, kRegCr1, kCr1Reset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVrReset[] = {
    {RegOp::Write8, kRegCr1, kCr1Reset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vr_rhine() noexcept {
    return Descriptor{
        .name = "vr-rhine",
        .provenance = "hbsd/src/sys/dev/vr/if_vrreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorVia,
        .device_id = kPciDevRhine,
        .init_sequence = kVrInit,
        .reset_sequence = kVrReset,
    };
}

} // namespace pbsd::uda::vr
""",
    "uda/descriptors/sis.cppm": """module;
#include <cstdint>

export module pbsd.uda.sis;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sis/if_sisreg.h
export namespace pbsd::uda::sis {

inline constexpr std::uint32_t kRegCsr = 0x00;
inline constexpr std::uint32_t kCsrReset = 0x00000100;
inline constexpr std::uint32_t kCsrRxEnable = 0x00000004;
inline constexpr std::uint32_t kCsrTxEnable = 0x00000001;

inline constexpr std::uint16_t kPciVendorSiS = 0x1039;
inline constexpr std::uint16_t kPciDev900 = 0x0900;

inline constexpr RegInsn kSisInit[] = {
    {RegOp::Write32, kRegCsr, kCsrReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegCsr, kCsrRxEnable | kCsrTxEnable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSisReset[] = {
    {RegOp::Write32, kRegCsr, kCsrReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor sis_900() noexcept {
    return Descriptor{
        .name = "sis-900",
        .provenance = "hbsd/src/sys/dev/sis/if_sisreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorSiS,
        .device_id = kPciDev900,
        .init_sequence = kSisInit,
        .reset_sequence = kSisReset,
    };
}

} // namespace pbsd::uda::sis
""",
    "uda/descriptors/bce.cppm": """module;
#include <cstdint>

export module pbsd.uda.bce;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bce/if_bcereg.h
export namespace pbsd::uda::bce {

inline constexpr std::uint32_t kRegCpReset = 0x00;
inline constexpr std::uint32_t kCpReset = 0x00000020;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDevNetXtreme2 = 0x1639;

inline constexpr RegInsn kBceInit[] = {
    {RegOp::Write32, kRegCpReset, kCpReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kBceReset[] = {
    {RegOp::Write32, kRegCpReset, kCpReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bce_netxtreme2() noexcept {
    return Descriptor{
        .name = "bce-netxtreme2",
        .provenance = "hbsd/src/sys/dev/bce/if_bcereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDevNetXtreme2,
        .init_sequence = kBceInit,
        .reset_sequence = kBceReset,
    };
}

} // namespace pbsd::uda::bce
""",
    "uda/descriptors/nfe.cppm": """module;
#include <cstdint>

export module pbsd.uda.nfe;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/nfe/if_nfereg.h
export namespace pbsd::uda::nfe {

inline constexpr std::uint32_t kRegMacReset = 0x03c;
inline constexpr std::uint32_t kMacResetMagic = 0x00f3;

inline constexpr std::uint16_t kPciVendorNvidia = 0x10DE;
inline constexpr std::uint16_t kPciDevMcp55Lan = 0x0373;

inline constexpr RegInsn kNfeInit[] = {
    {RegOp::Write32, kRegMacReset, kMacResetMagic, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNfeReset[] = {
    {RegOp::Write32, kRegMacReset, kMacResetMagic, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nfe_mcp55() noexcept {
    return Descriptor{
        .name = "nfe-mcp55",
        .provenance = "hbsd/src/sys/dev/nfe/if_nfereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorNvidia,
        .device_id = kPciDevMcp55Lan,
        .init_sequence = kNfeInit,
        .reset_sequence = kNfeReset,
    };
}

} // namespace pbsd::uda::nfe
""",
    "uda/descriptors/jme.cppm": """module;
#include <cstdint>

export module pbsd.uda.jme;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/jme/if_jmereg.h
export namespace pbsd::uda::jme {

inline constexpr std::uint32_t kRegSmi = 0x0050;
inline constexpr std::uint32_t kGhcReset = 0x40000000;

inline constexpr std::uint16_t kPciVendorJmicron = 0x197B;
inline constexpr std::uint16_t kPciDevJmc250 = 0x0250;

inline constexpr RegInsn kJmeInit[] = {
    {RegOp::Write32, kRegSmi, kGhcReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kJmeReset[] = {
    {RegOp::Write32, kRegSmi, kGhcReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor jme_jmc250() noexcept {
    return Descriptor{
        .name = "jme-jmc250",
        .provenance = "hbsd/src/sys/dev/jme/if_jmereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorJmicron,
        .device_id = kPciDevJmc250,
        .init_sequence = kJmeInit,
        .reset_sequence = kJmeReset,
    };
}

} // namespace pbsd::uda::jme
""",
    "uda/descriptors/vge.cppm": """module;
#include <cstdint>

export module pbsd.uda.vge;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vge/if_vgereg.h
export namespace pbsd::uda::vge {

inline constexpr std::uint32_t kRegCrs1 = 0x09;
inline constexpr std::uint8_t  kCr1SoftReset = 0x80;

inline constexpr std::uint16_t kPciVendorVIA = 0x1106;
inline constexpr std::uint16_t kPciDevRhine2 = 0x3106;

inline constexpr RegInsn kVgeInit[] = {
    {RegOp::Write8, kRegCrs1, kCr1SoftReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVgeReset[] = {
    {RegOp::Write8, kRegCrs1, kCr1SoftReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vge_rhine2() noexcept {
    return Descriptor{
        .name = "vge-rhine2",
        .provenance = "hbsd/src/sys/dev/vge/if_vgereg.h",
        .device_class = DeviceClass::Network,
        .device_id = kPciDevRhine2,
        .vendor_id = kPciVendorVia,
        .init_sequence = kVgeInit,
        .reset_sequence = kVgeReset,
    };
}

} // namespace pbsd::uda::vge
""",
    "uda/descriptors/bwn.cppm": """module;
#include <cstdint>

export module pbsd.uda.bwn;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/bwn/if_bwnreg.h
export namespace pbsd::uda::bwn {

inline constexpr std::uint32_t kIoctlPhyReset = 0x0008;
inline constexpr std::uint32_t kIoctlPhyClockEnable = 0x0004;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDevBcm4306 = 0x4306;

inline constexpr RegInsn kBwnInit[] = {
    {RegOp::Write32, 0x00, kIoctlPhyClockEnable, 0, 0},
    {RegOp::Write32, 0x00, kIoctlPhyReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bwn_bcm4306() noexcept {
    return Descriptor{
        .name = "bwn-bcm4306",
        .provenance = "hbsd/src/sys/dev/bwn/if_bwnreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDevBcm4306,
        .init_sequence = kBwnInit,
        .reset_sequence = kBwnInit,
    };
}

} // namespace pbsd::uda::bwn
""",
    "uda/descriptors/rum.cppm": """module;
#include <cstdint>

export module pbsd.uda.rum;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/wlan/if_rumreg.h
export namespace pbsd::uda::rum {

inline constexpr std::uint32_t kRegMacCtl = 0x100;
inline constexpr std::uint32_t kMacCtlReset = 0x00000001;

inline constexpr std::uint16_t kUsbVendorRalink = 0x148F;
inline constexpr std::uint16_t kUsbDev2573 = 0x2573;

inline constexpr RegInsn kRumInit[] = {
    {RegOp::Write32, kRegMacCtl, kMacCtlReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor rum_rt2573() noexcept {
    return Descriptor{
        .name = "rum-rt2573",
        .provenance = "hbsd/src/sys/dev/usb/wlan/if_rumreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kUsbVendorRalink,
        .device_id = kUsbDev2573,
        .init_sequence = kRumInit,
        .reset_sequence = kRumInit,
    };
}

} // namespace pbsd::uda::rum
""",
    "uda/descriptors/wpi.cppm": """module;
#include <cstdint>

export module pbsd.uda.wpi;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/wpi/if_wpireg.h
export namespace pbsd::uda::wpi {

inline constexpr std::uint32_t kRegReset = 0x020;
inline constexpr std::uint32_t kResetSw = 0x00000080;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev4965agn = 0x4222;

inline constexpr RegInsn kWpiInit[] = {
    {RegOp::Write32, kRegReset, kResetSw, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor wpi_4965agn() noexcept {
    return Descriptor{
        .name = "wpi-4965agn",
        .provenance = "hbsd/src/sys/dev/wpi/if_wpireg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev4965agn,
        .init_sequence = kWpiInit,
        .reset_sequence = kWpiInit,
    };
}

} // namespace pbsd::uda::wpi
""",
    "uda/descriptors/mwl.cppm": """module;
#include <cstdint>

export module pbsd.uda.mwl;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/mwl/mwlreg.h
export namespace pbsd::uda::mwl {

inline constexpr unsigned kMcastMax = 32;
inline constexpr std::uint32_t kRegReset = 0x00;
inline constexpr std::uint32_t kResetMagic = 0x00000001;

inline constexpr std::uint16_t kPciVendorMarvell = 0x11AB;
inline constexpr std::uint16_t kPciDev88w8363 = 0x1aAA;

inline constexpr RegInsn kMwlInit[] = {
    {RegOp::Write32, kRegReset, kResetMagic, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mwl_88w8363() noexcept {
    return Descriptor{
        .name = "mwl-88w8363",
        .provenance = "hbsd/src/sys/dev/mwl/mwlreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorMarvell,
        .device_id = kPciDev88w8363,
        .init_sequence = kMwlInit,
        .reset_sequence = kMwlInit,
    };
}

} // namespace pbsd::uda::mwl
""",
    "uda/descriptors/zyd.cppm": """module;
#include <cstdint>

export module pbsd.uda.zyd;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/wlan/if_zydreg.h
export namespace pbsd::uda::zyd {

inline constexpr std::uint32_t kRegMacCtl = 0x0100;
inline constexpr std::uint32_t kMacCtlReset = 0x00000001;

inline constexpr std::uint16_t kUsbVendorZydas = 0x0586;
inline constexpr std::uint16_t kUsbDev1211 = 0x3410;

inline constexpr RegInsn kZydInit[] = {
    {RegOp::Write32, kRegMacCtl, kMacCtlReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor zyd_zd1211() noexcept {
    return Descriptor{
        .name = "zyd-zd1211",
        .provenance = "hbsd/src/sys/dev/usb/wlan/if_zydreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kUsbVendorZydas,
        .device_id = kUsbDev1211,
        .init_sequence = kZydInit,
        .reset_sequence = kZydInit,
    };
}

} // namespace pbsd::uda::zyd
""",
}

DUPLICATE_PATHS = [
    PBSD / "kernel/sx/pbsd.kernel.sx.cppm",
    PBSD / "kernel/taskqueue/pbsd.kernel.taskqueue.cppm",
]

KERNEL_AGGREGATE_MISSING = [
    "pbsd_kernel_callout", "pbsd_kernel_tty", "pbsd_kernel_cons",
    "pbsd_kernel_uipc", "pbsd_kernel_sysvshm", "pbsd_kernel_sysvsem",
    "pbsd_kernel_sysvmsg", "pbsd_kernel_posix_shm", "pbsd_kernel_acct",
    "pbsd_kernel_racct", "pbsd_kernel_rctl", "pbsd_kernel_cpuset",
    "pbsd_kernel_numa", "pbsd_kernel_smr", "pbsd_kernel_epoch",
    "pbsd_kernel_timeout", "pbsd_kernel_intr", "pbsd_kernel_busdma",
]


def cmake_block(target: str, path: str, deps: list[str]) -> str:
    deps_lines = "\n".join(f"    {d}" for d in deps) if deps else ""
    link = f"target_link_libraries({target} PUBLIC\n{deps_lines})" if deps else ""
    return f"""add_library({target})
target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES
    {path})
{link}
target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})
"""


def patch_cmake(text: str) -> str:
    all_kernel = KERNEL_NEW + KERNEL_ORPHAN
    all_uda = UDA_NEW + UDA_ORPHAN

    # Insert kernel targets before pbsd_kernel INTERFACE
    kernel_insert = "\n".join(cmake_block(t, p, d) for t, p, d in all_kernel)
    marker = "add_library(pbsd_kernel INTERFACE)"
    if marker not in text:
        raise SystemExit("pbsd_kernel INTERFACE marker not found")
    if "pbsd_kernel_rmlock" not in text:
        text = text.replace(marker, kernel_insert + "\n" + marker)

    # Insert UDA targets before pbsd_uda INTERFACE
    uda_insert = "\n".join(cmake_block(t, p, d) for t, p, d in all_uda)
    uda_marker = "add_library(pbsd_uda INTERFACE)"
    if uda_marker not in text:
        raise SystemExit("pbsd_uda INTERFACE marker not found")
    if "pbsd_uda_vr" not in text:
        text = text.replace(uda_marker, uda_insert + "\n" + uda_marker)

    # Patch pbsd_kernel aggregate
    agg_start = "target_link_libraries(pbsd_kernel INTERFACE"
    agg_end = "target_compile_options(pbsd_kernel INTERFACE ${PBSD_FS_CXX})"
    start_idx = text.index(agg_start)
    end_idx = text.index(agg_end)
    agg_block = text[start_idx:end_idx]

    kernel_targets = [t for t, _, _ in all_kernel] + KERNEL_AGGREGATE_MISSING
    for t in kernel_targets:
        if t not in agg_block:
            # insert before pbsd_kernel_shim line or before closing paren
            if "    pbsd_kernel_shim)" in agg_block:
                agg_block = agg_block.replace(
                    "    pbsd_kernel_shim)",
                    f"    {t} pbsd_kernel_shim)",
                )
            else:
                agg_block = agg_block.replace(
                    "    pbsd_kernel_pax_aslr pbsd_kernel_shim)",
                    f"    pbsd_kernel_pax_aslr {t} pbsd_kernel_shim)",
                )

    text = text[:start_idx] + agg_block + text[end_idx:]

    # Patch pbsd_uda aggregate
    uda_agg_start = "target_link_libraries(pbsd_uda INTERFACE"
    uda_agg_end = "    pbsd_handles)"
    u_start = text.index(uda_agg_start)
    u_end = text.index(uda_agg_end, u_start) + len(uda_agg_end)
    uda_agg = text[u_start:u_end]

    uda_targets = [t for t, _, _ in all_uda]
    for t in uda_targets:
        if t not in uda_agg:
            uda_agg = uda_agg.replace(
                "    pbsd_handles)",
                f"    {t}\n    pbsd_handles)",
            )

    text = text[:u_start] + uda_agg + text[u_end:]
    return text


def main() -> int:
    for dup in DUPLICATE_PATHS:
        if dup.exists():
            dup.unlink()
            print(f"removed duplicate {dup.relative_to(ROOT)}")

    for rel, content in KERNEL_MODULES.items():
        path = PBSD / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {path.relative_to(ROOT)}")

    for rel, content in UDA_MODULES.items():
        path = PBSD / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {path.relative_to(ROOT)}")

    cmake = CMAKE.read_text(encoding="utf-8")
    cmake = patch_cmake(cmake)
    CMAKE.write_text(cmake, encoding="utf-8")
    print(f"patched {CMAKE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
