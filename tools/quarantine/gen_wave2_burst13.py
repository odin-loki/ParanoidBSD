#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Generate Burst 13 kernel subr/vfs + UDA descriptor hand-port modules."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PBSD = ROOT / "pbsd"
CMAKE = PBSD / "CMakeLists.txt"

KERNEL = [
    ("pbsd_kernel_hash", "kernel/kern/pbsd.kernel.hash.cppm", ["pbsd_core"]),
    ("pbsd_kernel_prf", "kernel/kern/pbsd.kernel.prf.cppm", ["pbsd_core"]),
    ("pbsd_kernel_unit", "kernel/kern/pbsd.kernel.unit.cppm", ["pbsd_core"]),
    ("pbsd_kernel_fattime", "kernel/kern/pbsd.kernel.fattime.cppm", ["pbsd_core"]),
    ("pbsd_kernel_msgbuf", "kernel/kern/pbsd.kernel.msgbuf.cppm", ["pbsd_core"]),
    ("pbsd_kernel_firmware", "kernel/kern/pbsd.kernel.firmware.cppm", ["pbsd_core"]),
    ("pbsd_kernel_pctrie", "kernel/kern/pbsd.kernel.pctrie.cppm", ["pbsd_core"]),
    ("pbsd_kernel_sglist", "kernel/kern/pbsd.kernel.sglist.cppm", ["pbsd_core"]),
    ("pbsd_kernel_sfbuf", "kernel/kern/pbsd.kernel.sfbuf.cppm", ["pbsd_core"]),
    ("pbsd_kernel_rman", "kernel/kern/pbsd.kernel.rman.cppm", ["pbsd_core"]),
    ("pbsd_kernel_blist", "kernel/kern/pbsd.kernel.blist.cppm", ["pbsd_core"]),
    ("pbsd_kernel_hints", "kernel/kern/pbsd.kernel.hints.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kern_log", "kernel/kern/pbsd.kernel.kern_log.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_mount", "kernel/vfs/pbsd.kernel.vfs_mount.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_subr", "kernel/vfs/pbsd.kernel.vfs_subr.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_lookup", "kernel/vfs/pbsd.kernel.vfs_lookup.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_default", "kernel/vfs/pbsd.kernel.vfs_default.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_syscalls", "kernel/vfs/pbsd.kernel.vfs_syscalls.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_vnops", "kernel/vfs/pbsd.kernel.vfs_vnops.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_hash", "kernel/vfs/pbsd.kernel.vfs_hash.cppm", ["pbsd_core"]),
    ("pbsd_kernel_vfs_cache", "kernel/vfs/pbsd.kernel.vfs_cache.cppm", ["pbsd_core"]),
    ("pbsd_kernel_synch", "kernel/kern/pbsd.kernel.synch.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kern_mib", "kernel/kern/pbsd.kernel.kern_mib.cppm", ["pbsd_kernel_sysctl"]),
    ("pbsd_kernel_kern_malloc", "kernel/kern/pbsd.kernel.kern_malloc.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kern_proc", "kernel/kern/pbsd.kernel.kern_proc.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kern_sig", "kernel/kern/pbsd.kernel.kern_sig.cppm", ["pbsd_kernel_signal"]),
    ("pbsd_kernel_kern_fork", "kernel/kern/pbsd.kernel.kern_fork.cppm", ["pbsd_core"]),
    ("pbsd_kernel_kern_exit", "kernel/kern/pbsd.kernel.kern_exit.cppm", ["pbsd_core"]),
]

UDA = [
    ("pbsd_uda_coretemp", "uda/descriptors/coretemp.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_amdtemp", "uda/descriptors/amdtemp.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_hid", "uda/descriptors/hid.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_evdev", "uda/descriptors/evdev.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_efidev", "uda/descriptors/efidev.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_age", "uda/descriptors/age.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_axgbe", "uda/descriptors/axgbe.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_enic", "uda/descriptors/enic.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_et", "uda/descriptors/et.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_clk", "uda/descriptors/clk.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_amdsbwd", "uda/descriptors/amdsbwd.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_amdgpio", "uda/descriptors/amdgpio.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_cyapa", "uda/descriptors/cyapa.cppm", ["pbsd_uda_schema", "pbsd_uda_interp"]),
    ("pbsd_uda_chromebook_platform", "uda/descriptors/chromebook_platform.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_cpuctl", "uda/descriptors/cpuctl.cppm", ["pbsd_uda_schema"]),
    ("pbsd_uda_amdsmb", "uda/descriptors/amdsmb.cppm", ["pbsd_uda_schema"]),
]

MODULES: dict[str, str] = {
    "kernel/kern/pbsd.kernel.hash.cppm": """module;
#include <cstdint>

export module pbsd.kernel.hash;

export import pbsd.core;

/// Freestanding port of `sys/hash.h` / `kern/subr_hash.c`.
export namespace pbsd::kernel::hash {

inline constexpr std::uint32_t kInit = 5381;

[[nodiscard]] constexpr std::uint32_t step(std::uint32_t h, unsigned char c) noexcept {
    return (h << 5) + h + static_cast<std::uint32_t>(c);
}

[[nodiscard]] inline std::uint32_t buf32(const unsigned char* p, std::size_t len,
                                         std::uint32_t seed = kInit) noexcept {
    if (p == nullptr) {
        return seed;
    }
    std::uint32_t h = seed;
    while (len-- > 0) {
        h = step(h, *p++);
    }
    return h;
}

[[nodiscard]] inline std::uint32_t str32(const char* s, std::uint32_t seed = kInit) noexcept {
    if (s == nullptr) {
        return seed;
    }
    std::uint32_t h = seed;
    while (*s != '\\0') {
        h = step(h, static_cast<unsigned char>(*s++));
    }
    return h;
}

} // namespace pbsd::kernel::hash
""",
    "kernel/kern/pbsd.kernel.prf.cppm": """module;

export module pbsd.kernel.prf;

export import pbsd.core;

/// Freestanding port of `kern/subr_prf.c` — console print flags.
export namespace pbsd::kernel::prf {

inline constexpr int kToLog   = 0x0001;
inline constexpr int kToCons  = 0x0002;
inline constexpr int kToKtr   = 0x0004;
inline constexpr int kToMsgbuf = 0x0008;
inline constexpr int kToStderr = 0x0010;

[[nodiscard]] constexpr bool to_console(int flags) noexcept {
    return (flags & kToCons) != 0;
}

[[nodiscard]] constexpr bool to_msgbuf(int flags) noexcept {
    return (flags & kToMsgbuf) != 0;
}

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0 || (flags & ~(kToLog | kToCons | kToKtr | kToMsgbuf | kToStderr)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::prf
""",
    "kernel/kern/pbsd.kernel.unit.cppm": """module;

export module pbsd.kernel.unit;

export import pbsd.core;

/// Freestanding port of `kern/subr_unit.c` — human-readable byte scaling.
export namespace pbsd::kernel::unit {

inline constexpr unsigned kKilo = 1024U;
inline constexpr unsigned kMega = kKilo * kKilo;
inline constexpr unsigned kGiga = kMega * kKilo;

enum class Scale : unsigned char { Bytes, Kilo, Mega, Giga };

[[nodiscard]] inline Status pick_scale(std::uint64_t bytes, Scale& out) noexcept {
    if (bytes >= static_cast<std::uint64_t>(kGiga)) {
        out = Scale::Giga;
    } else if (bytes >= static_cast<std::uint64_t>(kMega)) {
        out = Scale::Mega;
    } else if (bytes >= static_cast<std::uint64_t>(kKilo)) {
        out = Scale::Kilo;
    } else {
        out = Scale::Bytes;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t divisor(Scale s) noexcept {
    switch (s) {
    case Scale::Giga: return kGiga;
    case Scale::Mega: return kMega;
    case Scale::Kilo: return kKilo;
    default: return 1;
    }
}

} // namespace pbsd::kernel::unit
""",
    "kernel/kern/pbsd.kernel.fattime.cppm": """module;
#include <cstdint>

export module pbsd.kernel.fattime;

export import pbsd.core;

/// Freestanding port of `kern/subr_fattime.c` — FAT timestamp packing.
export namespace pbsd::kernel::fattime {

inline constexpr unsigned kSecondsPerMinute = 60;
inline constexpr unsigned kMinutesPerHour = 60;
inline constexpr unsigned kHoursPerDay = 24;
inline constexpr unsigned kDaysPerMonth = 31;
inline constexpr unsigned kMonthsPerYear = 12;
inline constexpr unsigned kYearBase = 1980;

[[nodiscard]] inline Status validate_fields(unsigned sec, unsigned min, unsigned hour,
                                            unsigned day, unsigned mon, unsigned year) noexcept {
    if (sec >= kSecondsPerMinute || min >= kMinutesPerHour || hour >= kHoursPerDay) {
        return Status::Invalid;
    }
    if (day == 0 || day > kDaysPerMonth || mon == 0 || mon > kMonthsPerYear) {
        return Status::Invalid;
    }
    if (year < kYearBase) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint32_t pack_time(unsigned sec, unsigned min, unsigned hour) noexcept {
    return (hour << 11) | (min << 5) | (sec / 2);
}

[[nodiscard]] inline std::uint16_t pack_date(unsigned day, unsigned mon, unsigned year) noexcept {
    return static_cast<std::uint16_t>(((year - kYearBase) << 9) | (mon << 5) | day);
}

} // namespace pbsd::kernel::fattime
""",
    "kernel/kern/pbsd.kernel.msgbuf.cppm": """module;
#include <cstdint>

export module pbsd.kernel.msgbuf;

export import pbsd.core;

/// Freestanding port of `sys/msgbuf.h` / `kern/subr_msgbuf.c`.
export namespace pbsd::kernel::msgbuf {

inline constexpr unsigned kMagic = 0x063062;
inline constexpr unsigned kFlagNeedNl = 0x01;
inline constexpr unsigned kFlagWrap = 0x02;

struct Softc {
    char*   ptr{nullptr};
    unsigned magic{};
    unsigned size{};
    unsigned wseq{};
    unsigned rseq{};
    unsigned seqmod{};
    unsigned flags{};
};

[[nodiscard]] inline unsigned seq_norm(const Softc& m, unsigned seq) noexcept {
    return (seq + m.seqmod) % m.seqmod;
}

[[nodiscard]] inline Status init(Softc& m, void* buf, unsigned size) noexcept {
    if (buf == nullptr || size == 0) {
        return Status::Invalid;
    }
    m.ptr = static_cast<char*>(buf);
    m.size = size;
    m.seqmod = size + 1;
    m.magic = kMagic;
    m.wseq = 0;
    m.rseq = 0;
    m.flags = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status add_char(Softc& m, int c) noexcept {
    if (m.magic != kMagic || m.ptr == nullptr) {
        return Status::Invalid;
    }
    const unsigned pos = m.wseq % m.size;
    m.ptr[pos] = static_cast<char>(c);
    m.wseq = seq_norm(m, m.wseq + 1);
    if (m.wseq == 0) {
        m.flags |= kFlagWrap;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::msgbuf
""",
    "kernel/kern/pbsd.kernel.firmware.cppm": """module;
#include <cstdint>

export module pbsd.kernel.firmware;

export import pbsd.core;

/// Freestanding port of `sys/firmware.h` / `kern/subr_firmware.c`.
export namespace pbsd::kernel::firmware {

inline constexpr unsigned kGetNoWarn = 0x0001;
inline constexpr unsigned kUnload = 0x0001;

struct Image {
    const char* name{nullptr};
    const void* data{nullptr};
    std::size_t size{};
    unsigned version{};
};

[[nodiscard]] inline Status validate_image(const Image& img) noexcept {
    if (img.name == nullptr || *img.name == '\\0') {
        return Status::Invalid;
    }
    if (img.data == nullptr || img.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool no_warn(unsigned flags) noexcept {
    return (flags & kGetNoWarn) != 0;
}

} // namespace pbsd::kernel::firmware
""",
    "kernel/kern/pbsd.kernel.pctrie.cppm": """module;
#include <cstdint>

export module pbsd.kernel.pctrie;

export import pbsd.core;

/// Freestanding port of `sys/pctrie.h` / `kern/subr_pctrie.c`.
export namespace pbsd::kernel::pctrie {

struct Iter {
    void*  tree{nullptr};
    void*  node{nullptr};
    std::uint64_t index{};
    std::uint64_t limit{};
};

inline void reset(Iter& it) noexcept { it.node = nullptr; }

[[nodiscard]] inline bool is_reset(const Iter& it) noexcept { return it.node == nullptr; }

inline void init(Iter& it, void* tree) noexcept {
    it.tree = tree;
    it.node = nullptr;
    it.limit = 0;
}

[[nodiscard]] inline Status validate_key(std::uint64_t key) noexcept {
    (void)key;
    return Status::Ok;
}

} // namespace pbsd::kernel::pctrie
""",
    "kernel/kern/pbsd.kernel.sglist.cppm": """module;
#include <cstdint>

export module pbsd.kernel.sglist;

export import pbsd.core;

/// Freestanding port of `sys/sglist.h` / `kern/subr_sglist.c`.
export namespace pbsd::kernel::sglist {

struct Seg {
    std::uint64_t paddr{};
    std::size_t   len{};
};

struct List {
    Seg*      segs{nullptr};
    unsigned  refs{1};
    unsigned short nseg{};
    unsigned short maxseg{};
};

inline void init(List& sg, unsigned short maxsegs, Seg* segs) noexcept {
    sg.segs = segs;
    sg.nseg = 0;
    sg.maxseg = maxsegs;
    sg.refs = 1;
}

inline void reset(List& sg) noexcept { sg.nseg = 0; }

[[nodiscard]] inline Status append(List& sg, std::uint64_t paddr, std::size_t len) noexcept {
    if (sg.segs == nullptr || sg.nseg >= sg.maxseg || len == 0) {
        return Status::Invalid;
    }
    sg.segs[sg.nseg].paddr = paddr;
    sg.segs[sg.nseg].len = len;
    ++sg.nseg;
    return Status::Ok;
}

} // namespace pbsd::kernel::sglist
""",
    "kernel/kern/pbsd.kernel.sfbuf.cppm": """module;
#include <cstdint>

export module pbsd.kernel.sfbuf;

export import pbsd.core;

/// Freestanding port of `sys/sfbuf.h` / `kern/subr_sfbuf.c`.
export namespace pbsd::kernel::sfbuf {

inline constexpr unsigned kFlagRead = 0x0001;
inline constexpr unsigned kFlagWrite = 0x0002;
inline constexpr unsigned kFlagUnmapped = 0x0004;

struct Softc {
    void*     va{nullptr};
    std::size_t len{};
    unsigned  flags{};
};

[[nodiscard]] inline bool is_read(unsigned flags) noexcept {
    return (flags & kFlagRead) != 0;
}

[[nodiscard]] inline Status validate(Softc& s) noexcept {
    if (s.va == nullptr || s.len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::sfbuf
""",
    "kernel/kern/pbsd.kernel.rman.cppm": """module;
#include <cstdint>

export module pbsd.kernel.rman;

export import pbsd.core;

/// Freestanding port of `sys/rman.h` / `kern/subr_rman.c`.
export namespace pbsd::kernel::rman {

inline constexpr unsigned kShareable = 0x01;
inline constexpr unsigned kSparse = 0x02;

struct Region {
    std::uint64_t start{};
    std::uint64_t end{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_region(const Region& r) noexcept {
    if (r.end < r.start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t size(const Region& r) noexcept {
    return r.end - r.start + 1;
}

} // namespace pbsd::kernel::rman
""",
    "kernel/kern/pbsd.kernel.blist.cppm": """module;
#include <cstdint>

export module pbsd.kernel.blist;

export import pbsd.core;

/// Freestanding port of `sys/blist.h` / `kern/subr_blist.c`.
export namespace pbsd::kernel::blist {

inline constexpr unsigned kMetaSize = 64;

struct Softc {
    unsigned* map{nullptr};
    unsigned  blocks{};
    unsigned  shift{};
};

[[nodiscard]] inline Status init(Softc& b, unsigned blocks) noexcept {
    if (blocks == 0) {
        return Status::Invalid;
    }
    b.blocks = blocks;
    b.shift = 5;
    return Status::Ok;
}

[[nodiscard]] inline unsigned block_index(const Softc& b, unsigned blk) noexcept {
    return blk >> b.shift;
}

} // namespace pbsd::kernel::blist
""",
    "kernel/kern/pbsd.kernel.hints.cppm": """module;

export module pbsd.kernel.hints;

export import pbsd.core;

/// Freestanding port of `kern/subr_hints.c` — loader hints KPI.
export namespace pbsd::kernel::hints {

inline constexpr unsigned kMaxName = 64;
inline constexpr unsigned kMaxValue = 256;

struct Entry {
    const char* name{nullptr};
    const char* value{nullptr};
};

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.name == nullptr || *e.name == '\\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::hints
""",
    "kernel/kern/pbsd.kernel.kern_log.cppm": """module;

export module pbsd.kernel.kern_log;

export import pbsd.core;

/// Freestanding port of `kern/subr_log.c` — log(9) priorities.
export namespace pbsd::kernel::kern_log {

inline constexpr int kEmerg = 0;
inline constexpr int kAlert = 1;
inline constexpr int kCrit = 2;
inline constexpr int kErr = 3;
inline constexpr int kWarning = 4;
inline constexpr int kNotice = 5;
inline constexpr int kInfo = 6;
inline constexpr int kDebug = 7;

[[nodiscard]] inline Status validate_pri(int pri) noexcept {
    if (pri < kEmerg || pri > kDebug) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* pri_name(int pri) noexcept {
    switch (pri) {
    case kEmerg: return "emerg";
    case kAlert: return "alert";
    case kCrit: return "crit";
    case kErr: return "err";
    case kWarning: return "warning";
    case kNotice: return "notice";
    case kInfo: return "info";
    case kDebug: return "debug";
    default: return nullptr;
    }
}

} // namespace pbsd::kernel::kern_log
""",
    "kernel/vfs/pbsd.kernel.vfs_mount.cppm": """module;
#include <cstdint>

export module pbsd.kernel.vfs_mount;

export import pbsd.core;

/// Freestanding port of `sys/mount.h` / `kern/vfs_mount.c`.
export namespace pbsd::kernel::vfs_mount {

inline constexpr unsigned kFsNameLen = 16;
inline constexpr unsigned kMountNameLen = 1024;
inline constexpr unsigned kStatFsVersion = 0x20140518;

inline constexpr std::uint64_t kRdOnly = 0x0000000000000001ULL;
inline constexpr std::uint64_t kSync = 0x0000000000000002ULL;
inline constexpr std::uint64_t kNoExec = 0x0000000000000004ULL;
inline constexpr std::uint64_t kNoSuid = 0x0000000000000008ULL;
inline constexpr std::uint64_t kAsync = 0x0000000000000040ULL;
inline constexpr std::uint64_t kLocal = 0x0000000000001000ULL;
inline constexpr std::uint64_t kNoAtime = 0x0000000010000000ULL;

struct Fsid {
    int val[2]{};
};

[[nodiscard]] inline bool fsid_cmp(const Fsid& a, const Fsid& b) noexcept {
    return a.val[0] != b.val[0] || a.val[1] != b.val[1];
}

[[nodiscard]] inline bool has_flag(std::uint64_t flags, std::uint64_t bit) noexcept {
    return (flags & bit) != 0;
}

[[nodiscard]] inline Status validate_flags(std::uint64_t flags) noexcept {
    (void)flags;
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_mount
""",
    "kernel/vfs/pbsd.kernel.vfs_subr.cppm": """module;

export module pbsd.kernel.vfs_subr;

export import pbsd.core;
import pbsd.kernel.vfs_mount;

/// Freestanding port of `kern/vfs_subr.c` — mount list helpers.
export namespace pbsd::kernel::vfs_subr {

inline constexpr unsigned kRootMount = 0x0001;
inline constexpr unsigned kForce = 0x0002;

[[nodiscard]] inline bool is_root_mount(unsigned flags) noexcept {
    return (flags & kRootMount) != 0;
}

[[nodiscard]] inline Status validate_unmount_flags(unsigned flags) noexcept {
    if (flags & ~(kRootMount | kForce)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_subr
""",
    "kernel/vfs/pbsd.kernel.vfs_lookup.cppm": """module;

export module pbsd.kernel.vfs_lookup;

export import pbsd.core;
import pbsd.kernel.namei;

/// Freestanding port of `kern/vfs_lookup.c` — namei lookup states.
export namespace pbsd::kernel::vfs_lookup {

inline constexpr unsigned kLookupShared = 0x0001;
inline constexpr unsigned kLookupRoot = 0x0002;
inline constexpr unsigned kLookupCrossmount = 0x0004;

[[nodiscard]] inline bool is_shared(unsigned flags) noexcept {
    return (flags & kLookupShared) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kLookupShared | kLookupRoot | kLookupCrossmount)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_lookup
""",
    "kernel/vfs/pbsd.kernel.vfs_default.cppm": """module;

export module pbsd.kernel.vfs_default;

export import pbsd.core;

/// Freestanding port of `kern/vfs_default.c` — default vnode ops stubs.
export namespace pbsd::kernel::vfs_default {

enum class Op : unsigned char {
    Open, Close, Read, Write, Ioctl, Poll, Kqfilter, Revoke, Mmap, Fsync,
    Seek, Remove, Link, Rename, Mkdir, Rmdir, Readdir, Symlink, Readlink,
    Abortop, Access, Getattr, Setattr, Pathconf, Advlock, Reallocblks,
    Strategy, Getwritemount, Suspended, Reclaim, Inactive, Needinval,
    Advlockasync, Vptofh, Vptocnp, Truncate, Copy_file_range, Offload,
    Aio_read, Aio_write, Aio_fsync, Aio_cancel, Aio_error, Aio_return,
    Unknown,
};

[[nodiscard]] inline Status default_not_supported(Op op) noexcept {
    (void)op;
    return Status::Denied;
}

} // namespace pbsd::kernel::vfs_default
""",
    "kernel/vfs/pbsd.kernel.vfs_syscalls.cppm": """module;

export module pbsd.kernel.vfs_syscalls;

export import pbsd.core;
import pbsd.kernel.vfs_mount;

/// Freestanding port of `kern/vfs_syscalls.c` — mount/open flags.
export namespace pbsd::kernel::vfs_syscalls {

inline constexpr int kO_RDONLY = 0;
inline constexpr int kO_WRONLY = 1;
inline constexpr int kO_RDWR = 2;
inline constexpr int kO_CREAT = 0x0200;
inline constexpr int kO_EXCL = 0x0800;
inline constexpr int kO_TRUNC = 0x0400;

[[nodiscard]] inline bool wants_create(int flags) noexcept {
    return (flags & kO_CREAT) != 0;
}

[[nodiscard]] inline Status validate_open_flags(int flags) noexcept {
    const int acc = flags & kO_RDWR;
    if (acc != kO_RDONLY && acc != kO_WRONLY && acc != kO_RDWR) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_syscalls
""",
    "kernel/vfs/pbsd.kernel.vfs_vnops.cppm": """module;

export module pbsd.kernel.vfs_vnops;

export import pbsd.core;
import pbsd.kernel.vnode;

/// Freestanding port of `kern/vfs_vnops.c` — vn_open/vn_close helpers.
export namespace pbsd::kernel::vfs_vnops {

inline constexpr unsigned kVnShared = 0x0001;
inline constexpr unsigned kVnNoFail = 0x0002;

[[nodiscard]] inline Status validate_vn_flags(unsigned flags) noexcept {
    if (flags & ~(kVnShared | kVnNoFail)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_vnops
""",
    "kernel/vfs/pbsd.kernel.vfs_hash.cppm": """module;
#include <cstdint>

export module pbsd.kernel.vfs_hash;

export import pbsd.core;
import pbsd.kernel.hash;

/// Freestanding port of `kern/vfs_hash.c` — vnode hash buckets.
export namespace pbsd::kernel::vfs_hash {

inline constexpr unsigned kBucketCount = 256;

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash) noexcept {
    return hash % kBucketCount;
}

[[nodiscard]] inline std::uint32_t vnode_hash(std::uint64_t ino, unsigned gen) noexcept {
    return pbsd::kernel::hash::step(
        pbsd::kernel::hash::step(pbsd::kernel::hash::kInit,
                                 static_cast<unsigned char>(ino & 0xff)),
        static_cast<unsigned char>(gen & 0xff));
}

} // namespace pbsd::kernel::vfs_hash
""",
    "kernel/vfs/pbsd.kernel.vfs_cache.cppm": """module;

export module pbsd.kernel.vfs_cache;

export import pbsd.core;

/// Freestanding port of `kern/vfs_cache.c` — name cache flags.
export namespace pbsd::kernel::vfs_cache {

inline constexpr unsigned kCacheFree = 0x0001;
inline constexpr unsigned kCacheDead = 0x0002;
inline constexpr unsigned kCacheNegative = 0x0004;

[[nodiscard]] inline bool is_negative(unsigned flags) noexcept {
    return (flags & kCacheNegative) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (flags & ~(kCacheFree | kCacheDead | kCacheNegative)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_cache
""",
    "kernel/kern/pbsd.kernel.synch.cppm": """module;

export module pbsd.kernel.synch;

export import pbsd.core;

/// Freestanding port of `kern/kern_synch.c` — sleep/wakeup flags.
export namespace pbsd::kernel::synch {

inline constexpr int kSleepPri = 0;
inline constexpr int kSleepIntr = 0x0100;
inline constexpr int kSleepCatch = 0x0200;
inline constexpr int kSleepFail = 0x0400;

[[nodiscard]] inline bool is_interruptible(int flags) noexcept {
    return (flags & kSleepIntr) != 0;
}

[[nodiscard]] inline Status validate_sleep_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::synch
""",
    "kernel/kern/pbsd.kernel.kern_mib.cppm": """module;

export module pbsd.kernel.kern_mib;

export import pbsd.core;
import pbsd.kernel.sysctl;

/// Freestanding port of `kern/kern_mib.c` — CTL_KERN OID subset.
export namespace pbsd::kernel::kern_mib {

inline constexpr int kKernOstype = 1;
inline constexpr int kKernOsrelease = 2;
inline constexpr int kKernVersion = 3;
inline constexpr int kKernMaxvnodes = 5;
inline constexpr int kKernMaxproc = 6;
inline constexpr int kKernMaxfiles = 7;
inline constexpr int kKernArgmax = 8;
inline constexpr int kKernSecurelevel = 9;
inline constexpr int kKernHostname = 10;
inline constexpr int kKernHostid = 11;
inline constexpr int kKernClockrate = 12;
inline constexpr int kKernProc = 14;

struct KernOid {
    int number;
    const char* name;
};

inline constexpr KernOid kTable[] = {
    {kKernOstype, "ostype"},
    {kKernOsrelease, "osrelease"},
    {kKernVersion, "version"},
    {kKernMaxproc, "maxproc"},
    {kKernMaxfiles, "maxfiles"},
    {kKernHostname, "hostname"},
    {kKernProc, "proc"},
};

[[nodiscard]] inline const char* lookup_name(int num) noexcept {
    for (const auto& e : kTable) {
        if (e.number == num) {
            return e.name;
        }
    }
    return nullptr;
}

} // namespace pbsd::kernel::kern_mib
""",
    "kernel/kern/pbsd.kernel.kern_malloc.cppm": """module;
#include <cstdint>

export module pbsd.kernel.kern_malloc;

export import pbsd.core;

/// Freestanding port of `kern/kern_malloc.c` — malloc type flags.
export namespace pbsd::kernel::kern_malloc {

inline constexpr unsigned kWaitOk = 0x0001;
inline constexpr unsigned kNowait = 0x0002;
inline constexpr unsigned kZero = 0x0100;
inline constexpr unsigned kNoFree = 0x0200;

[[nodiscard]] inline bool can_wait(unsigned flags) noexcept {
    return (flags & kWaitOk) != 0 && (flags & kNowait) == 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & kWaitOk) && (flags & kNowait)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_malloc
""",
    "kernel/kern/pbsd.kernel.kern_proc.cppm": """module;

export module pbsd.kernel.kern_proc;

export import pbsd.core;

/// Freestanding port of `kern/kern_proc.c` — proc sysctl flags.
export namespace pbsd::kernel::kern_proc {

inline constexpr int kProcAll = 0;
inline constexpr int kProcByPid = 1;
inline constexpr int kProcByPgrp = 2;
inline constexpr int kProcBySession = 3;
inline constexpr int kProcByTgid = 4;

[[nodiscard]] inline Status validate_what(int what) noexcept {
    if (what < kProcAll || what > kProcByTgid) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_proc
""",
    "kernel/kern/pbsd.kernel.kern_sig.cppm": """module;

export module pbsd.kernel.kern_sig;

export import pbsd.core;
import pbsd.kernel.signal;

/// Freestanding port of `kern/kern_sig.c` — signal delivery flags.
export namespace pbsd::kernel::kern_sig {

inline constexpr int kSigCantIgn = 0x0001;
inline constexpr int kSigReset = 0x0002;
inline constexpr int kSigCore = 0x0004;

[[nodiscard]] inline bool cant_ignore(int flags) noexcept {
    return (flags & kSigCantIgn) != 0;
}

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_sig
""",
    "kernel/kern/pbsd.kernel.kern_fork.cppm": """module;

export module pbsd.kernel.kern_fork;

export import pbsd.core;

/// Freestanding port of `kern/kern_fork.c` — fork flags.
export namespace pbsd::kernel::kern_fork {

inline constexpr int kRfproc = 0x0001;
inline constexpr int kRfnowait = 0x0002;
inline constexpr int kRffdg = 0x0004;
inline constexpr int kRfmem = 0x0008;
inline constexpr int kRfposix = 0x0010;

[[nodiscard]] inline Status validate_flags(int flags) noexcept {
    if (flags < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool returns_child_pid(int flags) noexcept {
    return (flags & kRfproc) != 0;
}

} // namespace pbsd::kernel::kern_fork
""",
    "kernel/kern/pbsd.kernel.kern_exit.cppm": """module;

export module pbsd.kernel.kern_exit;

export import pbsd.core;

/// Freestanding port of `kern/kern_exit.c` — exit/wait status encoding.
export namespace pbsd::kernel::kern_exit {

[[nodiscard]] inline int encode_exit(int code) noexcept {
    return (code & 0xff) << 8;
}

[[nodiscard]] inline int encode_signal(int sig) noexcept {
    return sig;
}

[[nodiscard]] inline bool stopped(int status) noexcept {
    return (status & 0xff) == 0x7f;
}

[[nodiscard]] inline bool exited(int status) noexcept {
    return (status & 0xff) == 0;
}

[[nodiscard]] inline int exit_code(int status) noexcept {
    return (status >> 8) & 0xff;
}

} // namespace pbsd::kernel::kern_exit
""",
    "uda/descriptors/coretemp.cppm": """module;
#include <cstdint>

export module pbsd.uda.coretemp;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/coretemp/coretemp.c
export namespace pbsd::uda::coretemp {

inline constexpr unsigned kTzZeroC = 2731;
inline constexpr unsigned kThermStatus = 0x01;
inline constexpr unsigned kThermStatusLog = 0x02;
inline constexpr unsigned kThermCritical = 0x10;
inline constexpr unsigned kTempShift = 16;
inline constexpr unsigned kTempMask = 0x7f;

[[nodiscard]] inline int decode_celsius(std::uint32_t msr) noexcept {
    const unsigned raw = (msr >> kTempShift) & kTempMask;
    return static_cast<int>(raw);
}

[[nodiscard]] inline constexpr Descriptor intel_coretemp() noexcept {
    return Descriptor{
        .name = "coretemp",
        .provenance = "hbsd/src/sys/dev/coretemp/coretemp.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x8086,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::coretemp
""",
    "uda/descriptors/amdtemp.cppm": """module;
#include <cstdint>

export module pbsd.uda.amdtemp;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdtemp/amdtemp.c
export namespace pbsd::uda::amdtemp {

inline constexpr unsigned kOffsetC = 49;
inline constexpr unsigned kMask = 0xff;

[[nodiscard]] inline int decode_offset(std::uint32_t reg) noexcept {
    return static_cast<int>((reg >> 21) & kMask) - static_cast<int>(kOffsetC);
}

[[nodiscard]] inline constexpr Descriptor amd_k8_temp() noexcept {
    return Descriptor{
        .name = "amdtemp-k8",
        .provenance = "hbsd/src/sys/dev/amdtemp/amdtemp.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x1022,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::amdtemp
""",
    "uda/descriptors/hid.cppm": """module;
#include <cstdint>

export module pbsd.uda.hid;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hid/hid.c
export namespace pbsd::uda::hid {

inline constexpr std::uint16_t kUsbClassHid = 0x03;
inline constexpr std::uint8_t kDescReport = 0x22;
inline constexpr std::uint8_t kDescPhysical = 0x23;

inline constexpr RegInsn kHidInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor usb_hid_generic() noexcept {
    return Descriptor{
        .name = "usb-hid",
        .provenance = "hbsd/src/sys/dev/hid/hid.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kHidInit,
        .reset_sequence = kHidInit,
    };
}

} // namespace pbsd::uda::hid
""",
    "uda/descriptors/evdev.cppm": """module;

export module pbsd.uda.evdev;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/evdev/evdev.c
export namespace pbsd::uda::evdev {

inline constexpr unsigned kMaxSlots = 10;
inline constexpr unsigned kBufSize = 64;

enum class EventType : unsigned short { Syn = 0, Key = 1, Rel = 2, Abs = 3 };

[[nodiscard]] inline constexpr Descriptor evdev_generic() noexcept {
    return Descriptor{
        .name = "evdev",
        .provenance = "hbsd/src/sys/dev/evdev/evdev.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::evdev
""",
    "uda/descriptors/efidev.cppm": """module;
#include <cstdint>

export module pbsd.uda.efidev;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/efidev/efidev.c
export namespace pbsd::uda::efidev {

inline constexpr std::uint32_t kGuidEndEntire = 0x0000000010000000ULL;
inline constexpr std::uint16_t kDevPathPci = 0x01;
inline constexpr std::uint16_t kDevPathAcpi = 0x02;

[[nodiscard]] inline constexpr Descriptor efi_runtime() noexcept {
    return Descriptor{
        .name = "efidev",
        .provenance = "hbsd/src/sys/dev/efidev/efidev.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::efidev
""",
    "uda/descriptors/age.cppm": """module;
#include <cstdint>

export module pbsd.uda.age;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/age/if_agereg.h
export namespace pbsd::uda::age {

inline constexpr std::uint32_t kRegMaster = 0x0000;
inline constexpr std::uint32_t kMasterReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint16_t kPciDev1700 = 0x1700;

inline constexpr RegInsn kAgeInit[] = {
    {RegOp::Write32, kRegMaster, kMasterReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor age_1700() noexcept {
    return Descriptor{
        .name = "age-1700",
        .provenance = "hbsd/src/sys/dev/age/if_agereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAmd,
        .device_id = kPciDev1700,
        .init_sequence = kAgeInit,
        .reset_sequence = kAgeInit,
    };
}

} // namespace pbsd::uda::age
""",
    "uda/descriptors/axgbe.cppm": """module;
#include <cstdint>

export module pbsd.uda.axgbe;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/axgbe/if_axgreg.h
export namespace pbsd::uda::axgbe {

inline constexpr std::uint32_t kRegDmaMode = 0x1000;
inline constexpr std::uint32_t kDmaSoftReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint16_t kPciDev1458 = 0x1458;

inline constexpr RegInsn kAxgbeInit[] = {
    {RegOp::Write32, kRegDmaMode, kDmaSoftReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor axgbe_10g() noexcept {
    return Descriptor{
        .name = "axgbe-10g",
        .provenance = "hbsd/src/sys/dev/axgbe/if_axgreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAmd,
        .device_id = kPciDev1458,
        .init_sequence = kAxgbeInit,
        .reset_sequence = kAxgbeInit,
    };
}

} // namespace pbsd::uda::axgbe
""",
    "uda/descriptors/enic.cppm": """module;
#include <cstdint>

export module pbsd.uda.enic;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/enic/if_enicreg.h
export namespace pbsd::uda::enic {

inline constexpr std::uint32_t kRegDevCmd = 0x0000;
inline constexpr std::uint32_t kDevCmdReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorCisco = 0x1137;
inline constexpr std::uint16_t kPciDev0043 = 0x0043;

inline constexpr RegInsn kEnicInit[] = {
    {RegOp::Write32, kRegDevCmd, kDevCmdReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor enic_vnic() noexcept {
    return Descriptor{
        .name = "enic-vnic",
        .provenance = "hbsd/src/sys/dev/enic/if_enicreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorCisco,
        .device_id = kPciDev0043,
        .init_sequence = kEnicInit,
        .reset_sequence = kEnicInit,
    };
}

} // namespace pbsd::uda::enic
""",
    "uda/descriptors/et.cppm": """module;
#include <cstdint>

export module pbsd.uda.et;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/et/if_etreg.h
export namespace pbsd::uda::et {

inline constexpr std::uint32_t kRegIntrMask = 0x0000;
inline constexpr std::uint32_t kIntrDisable = 0xFFFFFFFFu;
inline constexpr std::uint16_t kPciVendorAgere = 0x11C1;
inline constexpr std::uint16_t kPciDev5801 = 0x5801;

inline constexpr RegInsn kEtInit[] = {
    {RegOp::Write32, kRegIntrMask, kIntrDisable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor et_1310() noexcept {
    return Descriptor{
        .name = "et-1310",
        .provenance = "hbsd/src/sys/dev/et/if_etreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAgere,
        .device_id = kPciDev5801,
        .init_sequence = kEtInit,
        .reset_sequence = kEtInit,
    };
}

} // namespace pbsd::uda::et
""",
    "uda/descriptors/clk.cppm": """module;
#include <cstdint>

export module pbsd.uda.clk;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/clk/clk.c
export namespace pbsd::uda::clk {

inline constexpr unsigned kMaxParents = 4;
inline constexpr unsigned kMaxName = 32;

[[nodiscard]] inline constexpr Descriptor clk_root() noexcept {
    return Descriptor{
        .name = "clk-root",
        .provenance = "hbsd/src/sys/dev/clk/clk.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::clk
""",
    "uda/descriptors/amdsbwd.cppm": """module;
#include <cstdint>

export module pbsd.uda.amdsbwd;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdsbwd/amdsbwd.c
export namespace pbsd::uda::amdsbwd {

inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr unsigned kWatchdogSec = 60;

[[nodiscard]] inline constexpr Descriptor amd_sb_watchdog() noexcept {
    return Descriptor{
        .name = "amdsbwd",
        .provenance = "hbsd/src/sys/dev/amdsbwd/amdsbwd.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::amdsbwd
""",
    "uda/descriptors/amdgpio.cppm": """module;
#include <cstdint>

export module pbsd.uda.amdgpio;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdgpio/amdgpio.c
export namespace pbsd::uda::amdgpio {

inline constexpr std::uint32_t kRegPinCtrl = 0x00;
inline constexpr std::uint32_t kPinInput = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;

[[nodiscard]] inline constexpr Descriptor amd_fch_gpio() noexcept {
    return Descriptor{
        .name = "amdgpio",
        .provenance = "hbsd/src/sys/dev/amdgpio/amdgpio.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::amdgpio
""",
    "uda/descriptors/cyapa.cppm": """module;
#include <cstdint>

export module pbsd.uda.cyapa;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cyapa/cyapa.c
export namespace pbsd::uda::cyapa {

inline constexpr std::uint8_t kRegDevStatus = 0x00;
inline constexpr std::uint8_t kStatusOperational = 0x03;
inline constexpr std::uint16_t kI2cAddr = 0x67;

inline constexpr RegInsn kCyapaInit[] = {
    {RegOp::Write8, kRegDevStatus, kStatusOperational, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cyapa_gen3() noexcept {
    return Descriptor{
        .name = "cyapa-gen3",
        .provenance = "hbsd/src/sys/dev/cyapa/cyapa.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = kI2cAddr,
        .init_sequence = kCyapaInit,
        .reset_sequence = kCyapaInit,
    };
}

} // namespace pbsd::uda::cyapa
""",
    "uda/descriptors/chromebook_platform.cppm": """module;

export module pbsd.uda.chromebook_platform;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/chromebook_platform/chromebook_platform.c
export namespace pbsd::uda::chromebook_platform {

inline constexpr unsigned kMaxButtons = 4;

[[nodiscard]] inline constexpr Descriptor cros_platform() noexcept {
    return Descriptor{
        .name = "chromebook-platform",
        .provenance = "hbsd/src/sys/dev/chromebook_platform/chromebook_platform.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::chromebook_platform
""",
    "uda/descriptors/cpuctl.cppm": """module;

export module pbsd.uda.cpuctl;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/cpuctl/cpuctl.c
export namespace pbsd::uda::cpuctl {

inline constexpr unsigned kIoctlMsr = 0x80000001;
inline constexpr unsigned kIoctlCpuid = 0x80000002;

[[nodiscard]] inline constexpr Descriptor cpuctl_dev() noexcept {
    return Descriptor{
        .name = "cpuctl",
        .provenance = "hbsd/src/sys/dev/cpuctl/cpuctl.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::cpuctl
""",
    "uda/descriptors/amdsmb.cppm": """module;
#include <cstdint>

export module pbsd.uda.amdsmb;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdsmb/amdsmb.c
export namespace pbsd::uda::amdsmb {

inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint32_t kRegControl = 0x00;

[[nodiscard]] inline constexpr Descriptor amd_fch_smb() noexcept {
    return Descriptor{
        .name = "amdsmb",
        .provenance = "hbsd/src/sys/dev/amdsmb/amdsmb.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = nullptr,
        .reset_sequence = nullptr,
    };
}

} // namespace pbsd::uda::amdsmb
""",
}


def cmake_guard_block(target: str, path: str, deps: list[str]) -> str:
    deps_lines = "\n".join(f"    {d}" for d in deps)
    return f"""if(NOT TARGET {target})
add_library({target})
target_sources({target} PUBLIC FILE_SET CXX_MODULES FILES
    {path})
target_link_libraries({target} PUBLIC
{deps_lines})
target_compile_options({target} PUBLIC ${{PBSD_FS_CXX}})
endif()

"""


def patch_cmake(text: str) -> str:
    # Kernel targets before pbsd_kernel INTERFACE
    k_marker = "if(NOT TARGET pbsd_kernel_sys_getrandom)"
    k_block = "".join(cmake_guard_block(t, p, d) for t, p, d in KERNEL)
    if "pbsd_kernel_hash" not in text:
        idx = text.index(k_marker)
        text = text[:idx] + k_block + text[idx:]

    # UDA targets before pbsd_uda INTERFACE
    u_marker = "if(NOT TARGET pbsd_uda)\nadd_library(pbsd_uda INTERFACE)"
    u_block = "".join(cmake_guard_block(t, p, d) for t, p, d in UDA)
    if "pbsd_uda_coretemp" not in text:
        idx = text.index(u_marker)
        text = text[:idx] + u_block + text[idx:]

    # pbsd_kernel aggregate
    k_targets = [t for t, _, _ in KERNEL]
    agg_needle = "    pbsd_kernel_context pbsd_kernel_sys_getrandom)"
    k_agg = "\n".join(f"    {t.replace('pbsd_kernel_', 'pbsd_kernel_')}" for t in k_targets)
    # Use cmake target names directly
    k_agg = " ".join(k_targets)
    if "pbsd_kernel_hash" not in text.split("target_link_libraries(pbsd_kernel INTERFACE")[1].split("target_compile_options")[0]:
        text = text.replace(
            agg_needle,
            f"    pbsd_kernel_context pbsd_kernel_sys_getrandom\n    {' '.join(k_targets)})",
        )

    # pbsd_uda aggregate — insert before pbsd_handles
    u_targets = [t for t, _, _ in UDA]
    if "pbsd_uda_coretemp" not in text.split("target_link_libraries(pbsd_uda INTERFACE")[1].split("endif()")[0]:
        text = text.replace(
            "    pbsd_uda_dcons\n    pbsd_handles)",
            "    pbsd_uda_dcons\n    " + "\n    ".join(u_targets) + "\n    pbsd_handles)",
        )

    return text


def main() -> int:
    for rel, content in MODULES.items():
        path = PBSD / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        print(f"wrote {path.relative_to(ROOT)}")

    cmake = CMAKE.read_text(encoding="utf-8")
    cmake = patch_cmake(cmake)
    CMAKE.write_text(cmake, encoding="utf-8")
    print(f"patched {CMAKE.relative_to(ROOT)}")
    print(f"kernel modules: {len(KERNEL)}, uda modules: {len(UDA)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
