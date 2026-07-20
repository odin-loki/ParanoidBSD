#!/usr/bin/env python3
"""Generate Wave 6–9 pass-2 partition .cppm modules (30+ new)."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MODULES: dict[str, str] = {
    "pbsd/net/pbsd.net.mbuf.cppm": '''module;
#include <cstdint>

export module pbsd.net.mbuf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/mbuf.h — M_* mbuf flags.
export namespace pbsd::net::mbuf {

enum class Flag : unsigned int {
    Ext     = 0x00000001,
    Pkthdr  = 0x00000002,
    Eor     = 0x00000004,
    Rdonly  = 0x00000008,
    Bcast   = 0x00000010,
    Mcast   = 0x00000020,
    Promisc = 0x00000040,
    Vlantag = 0x00000080,
    Extpg   = 0x00000100,
    Nofree  = 0x00000200,
    Tstmp   = 0x00000400,
};

inline constexpr unsigned kCopyFlags =
    static_cast<unsigned>(Flag::Pkthdr) | static_cast<unsigned>(Flag::Eor)
    | static_cast<unsigned>(Flag::Rdonly) | static_cast<unsigned>(Flag::Bcast)
    | static_cast<unsigned>(Flag::Mcast) | static_cast<unsigned>(Flag::Promisc)
    | static_cast<unsigned>(Flag::Vlantag) | static_cast<unsigned>(Flag::Tstmp);

[[nodiscard]] inline bool has_flag(unsigned flags, Flag f) noexcept {
    return (flags & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (has_flag(flags, Flag::Ext) && has_flag(flags, Flag::Extpg)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::mbuf
''',
    "pbsd/net/pbsd.net.bpf.cppm": '''module;
#include <cstdint>

export module pbsd.net.bpf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/bpf.h — BIOC* ioctls and BPF alignment.
export namespace pbsd::net::bpf {

inline constexpr unsigned kRelease = 199606;
inline constexpr unsigned kAlignment = sizeof(long);

enum class Ioctl : unsigned int {
    Gblen       = 102,
    Setf        = 103,
    Flush       = 104,
    Promisc     = 105,
    Gdlt        = 106,
    Getif       = 107,
    Setif       = 108,
    Version     = 113,
    Immediate   = 112,
    Direction   = 118,
};

[[nodiscard]] inline constexpr unsigned wordalign(unsigned x) noexcept {
    return (x + (kAlignment - 1)) & ~(kAlignment - 1);
}

[[nodiscard]] inline Status validate_ioctl(Ioctl cmd) noexcept {
    if (static_cast<unsigned>(cmd) > static_cast<unsigned>(Ioctl::Direction)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::bpf
''',
    "pbsd/net/pbsd.net.igmp.cppm": '''module;
#include <cstdint>

export module pbsd.net.igmp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/igmp.h — IGMP message types.
export namespace pbsd::net::igmp {

inline constexpr unsigned kMinLen = 8;
inline constexpr unsigned kV3QueryMinLen = 12;

enum class Type : unsigned char {
    HostMembershipQuery     = 0x11,
    V1HostMembershipReport  = 0x12,
    Dvmrp                   = 0x13,
    Pim                     = 0x14,
    V2HostMembershipReport  = 0x16,
    HostLeaveMessage        = 0x17,
    MtraceReply             = 0x1e,
    MtraceQuery             = 0x1f,
    V3HostMembershipReport  = 0x22,
};

enum class FilterMode : unsigned char {
    DoNothing      = 0,
    ModeIsInclude  = 1,
    ModeIsExclude  = 2,
};

struct Header {
    unsigned char  type{};
    unsigned char  code{};
    unsigned short cksum{};
    unsigned       group{};
};

[[nodiscard]] inline constexpr std::size_t type_table_size() noexcept { return 9; }

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.type == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::igmp
''',
    "pbsd/net/pbsd.net.bridge.cppm": '''module;
#include <cstdint>

export module pbsd.net.bridge;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_bridgevar.h — bridge ioctl flags.
export namespace pbsd::net::bridge {

enum class Flag : unsigned int {
    Learning  = 0x0001,
    Discover  = 0x0002,
    Stp       = 0x0004,
    StpUser   = 0x0008,
    StpAuto   = 0x0010,
    StpP2p    = 0x0020,
    Private   = 0x0040,
    Age       = 0x0080,
    Ieee8021d = 0x0100,
};

enum class Span : unsigned char {
    Disabled = 0,
    Enabled  = 1,
    Auto     = 2,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::StpUser))
        && (flags & static_cast<unsigned>(Flag::StpAuto))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::bridge
''',
    "pbsd/net/pbsd.net.pf.cppm": '''module;
#include <cstdint>

export module pbsd.net.pf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfvar.h — PF scrub/state flags (subset).
export namespace pbsd::net::pf {

inline constexpr char kReservedAnchor[] = "_pf";

enum class ScrubFlag : unsigned char {
    Valid = 0x01,
};

enum class StateFlag : unsigned int {
    None       = 0,
    Floating   = 0x01,
    Divert     = 0x02,
    NoSync     = 0x04,
    Pflow      = 0x08,
    Block      = 0x10,
    Table      = 0x20,
    Translation = 0x40,
};

[[nodiscard]] inline Status validate_scrub(unsigned flags) noexcept {
    if (flags != 0 && (flags & static_cast<unsigned>(ScrubFlag::Valid)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::pf
''',
    "pbsd/fs/pbsd.fs.attr.cppm": '''module;
#include <cstdint>

export module pbsd.fs.attr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/extattr.h — EXTATTR_NAMESPACE_*.
export namespace pbsd::fs::attr {

enum class Namespace : unsigned int {
    Empty  = 0x00000000,
    User   = 0x00000001,
    System = 0x00000002,
};

inline constexpr unsigned kMaxNameLen = 255;

[[nodiscard]] inline Status validate_namespace(Namespace ns) noexcept {
    switch (ns) {
    case Namespace::Empty:
    case Namespace::User:
    case Namespace::System:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline const char* namespace_string(Namespace ns) noexcept {
    switch (ns) {
    case Namespace::Empty:  return "empty";
    case Namespace::User:   return "user";
    case Namespace::System: return "system";
    default:                return nullptr;
    }
}

} // namespace pbsd::fs::attr
''',
    "pbsd/fs/pbsd.fs.lockf.cppm": '''module;
#include <cstdint>

export module pbsd.fs.lockf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/fcntl.h — F_SETLK / lock types.
export namespace pbsd::fs::lockf {

enum class Type : unsigned short {
    Unlck = 0,
    Rdlck = 1,
    Wrlck = 2,
    WrlckWait = 3,
};

enum class Whence : unsigned short {
    Set = 0,
    Cur = 1,
    End = 2,
};

struct Flock {
    Type           type{};
    Whence         whence{};
    long long      start{};
    long long      len{};
};

[[nodiscard]] inline Status validate_flock(Flock const& fl) noexcept {
    if (fl.len < 0) {
        return Status::Invalid;
    }
    if (fl.type == Type::Unlck && fl.len != 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::lockf
''',
    "pbsd/fs/pbsd.fs.quota.cppm": '''module;
#include <cstdint>

export module pbsd.fs.quota;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/quota.h — dqblk quota types.
export namespace pbsd::fs::quota {

enum class Type : unsigned char {
    User    = 0,
    Group   = 1,
    Default = 2,
};

enum class Flag : unsigned int {
    Inode  = 0x01,
    Block  = 0x02,
    Limits = 0x04,
};

struct Dqblk {
    long long bhardlimit{};
    long long bsoftlimit{};
    long long curblocks{};
    long long ihardlimit{};
    long long isoftlimit{};
    long long curinodes{};
    long long btime{};
    long long itime{};
};

[[nodiscard]] inline Status validate_dqblk(Dqblk const& dq) noexcept {
    if (dq.bsoftlimit > dq.bhardlimit && dq.bhardlimit >= 0) {
        return Status::Invalid;
    }
    if (dq.isoftlimit > dq.ihardlimit && dq.ihardlimit >= 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::quota
''',
    "pbsd/fs/pbsd.fs.ufs.cppm": '''module;
#include <cstdint>

export module pbsd.fs.ufs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/inode.h — UFS file flags (subset).
export namespace pbsd::fs::ufs {

enum class InodeFlag : unsigned int {
    Nsnapshot = 0x00000001,
    Immutable = 0x00000002,
    Append    = 0x00000004,
    Nounlink  = 0x00000008,
    Archive   = 0x00000010,
    Opaque    = 0x00000020,
    Nodump    = 0x00000040,
};

enum class FileType : unsigned char {
    Vfifo  = 1,
    Vchr   = 2,
    Vdir   = 4,
    Vblk   = 6,
    Vreg   = 8,
    Vlnk   = 10,
    Vsock  = 12,
};

[[nodiscard]] inline bool is_dir(FileType t) noexcept {
    return t == FileType::Vdir;
}

[[nodiscard]] inline Status validate_inode_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(InodeFlag::Immutable))
        && (flags & static_cast<unsigned>(InodeFlag::Append))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ufs
''',
    "pbsd/geom/pbsd.geom.part.cppm": '''module;
#include <cstdint>

export module pbsd.geom.part;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/part/g_part.h — g_part probe priority / aliases.
export namespace pbsd::geom::part {

inline constexpr int kProbePriLow  = -10;
inline constexpr int kProbePriNorm = -5;
inline constexpr int kProbePriHigh = 0;

enum class Alias : unsigned int {
    AppleApfs     = 0,
    AppleBoot     = 1,
    AppleHfs      = 3,
    BiosBoot      = 10,
    Efi           = 11,
    Freebsd       = 12,
    FreebsdBoot   = 13,
    FreebsdSwap   = 14,
    FreebsdZfs    = 15,
    LinuxData     = 20,
    LinuxSwap     = 21,
    Mbr           = 30,
    Gpt           = 31,
};

[[nodiscard]] inline Status validate_probe_pri(int pri) noexcept {
    if (pri < kProbePriLow || pri > kProbePriHigh) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::part
''',
    "pbsd/geom/pbsd.geom.mirror.cppm": '''module;
#include <cstdint>

export module pbsd.geom.mirror;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/mirror/g_mirror.h — MIRROR class metadata.
export namespace pbsd::geom::mirror {

inline constexpr char kClassName[] = "MIRROR";
inline constexpr char kMagic[] = "GEOM::MIRROR";
inline constexpr unsigned kVersion = 4;

enum class Balance : unsigned char {
    None       = 0,
    RoundRobin = 1,
    Load       = 2,
    Prefer     = 3,
};

enum class Flag : unsigned int {
    NoFailureSync = 0x01,
    NoSync        = 0x02,
    Destroy       = 0x04,
};

[[nodiscard]] inline Status validate_balance(Balance b) noexcept {
    if (static_cast<unsigned char>(b) > static_cast<unsigned char>(Balance::Prefer)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::mirror
''',
    "pbsd/geom/pbsd.geom.label.cppm": '''module;
#include <cstdint>

export module pbsd.geom.label;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/label/g_label.h — GEOM label providers.
export namespace pbsd::geom::label {

enum class Provider : unsigned int {
    Msdosfs = 0,
    Gpt     = 1,
    Iso9660 = 2,
    Ufs     = 3,
    Ext2fs  = 4,
    Reiserfs = 5,
    Ntfs    = 6,
    Zfs     = 7,
};

struct LabelEntry {
    Provider    provider{};
    const char* prefix{};
};

inline constexpr LabelEntry kProviderTable[] = {
    {Provider::Msdosfs, "msdosfs"},
    {Provider::Gpt, "gpt"},
    {Provider::Iso9660, "cd9660"},
    {Provider::Ufs, "ufs"},
    {Provider::Ext2fs, "ext2fs"},
    {Provider::Ntfs, "ntfs"},
    {Provider::Zfs, "zfs"},
};

[[nodiscard]] inline constexpr std::size_t provider_table_size() noexcept {
    return sizeof(kProviderTable) / sizeof(kProviderTable[0]);
}

} // namespace pbsd::geom::label
''',
    "pbsd/geom/pbsd.geom.taste.cppm": '''module;
#include <cstdint>

export module pbsd.geom.taste;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom.h — G_TF_* taste flags.
export namespace pbsd::geom::taste {

enum class Flag : unsigned int {
    Normal      = 0,
    Insist      = 1,
    Transparent = 2,
};

[[nodiscard]] inline Status validate_flag(Flag f) noexcept {
    if (static_cast<unsigned>(f) > static_cast<unsigned>(Flag::Transparent)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_transparent(Flag f) noexcept {
    return f == Flag::Transparent;
}

} // namespace pbsd::geom::taste
''',
    "pbsd/zfs/pbsd.zfs.dmu.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.dmu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dmu.h — objset types.
export namespace pbsd::zfs::dmu {

enum class ObjsetType : unsigned char {
    None = 0,
    Meta = 1,
    Zil  = 2,
    Dnode = 3,
    Other = 4,
};

enum class TxType : unsigned char {
    Read  = 0,
    Write = 1,
    Free  = 2,
    Claim = 3,
};

[[nodiscard]] inline Status validate_objset_type(ObjsetType t) noexcept {
    if (static_cast<unsigned char>(t) > static_cast<unsigned char>(ObjsetType::Other)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dmu
''',
    "pbsd/zfs/pbsd.zfs.dnode.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.dnode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dnode.h — dnode types.
export namespace pbsd::zfs::dnode {

enum class Type : unsigned char {
    None     = 0,
    Meta     = 1,
    Directory = 2,
    File     = 3,
    Zap      = 4,
    Dnode    = 5,
};

enum class Flag : unsigned int {
    DirtyLink   = 0x0001,
    DirtyData   = 0x0002,
    DirtyMeta   = 0x0004,
    Evicted     = 0x0008,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    if (static_cast<unsigned char>(t) > static_cast<unsigned char>(Type::Dnode)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dnode
''',
    "pbsd/zfs/pbsd.zfs.vdev.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.vdev;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_impl.h — vdev states.
export namespace pbsd::zfs::vdev {

enum class State : unsigned char {
    Unknown  = 0,
    Closed   = 1,
    Offline  = 2,
    Removed  = 3,
    Faulted  = 4,
    Degraded = 5,
    Healthy  = 6,
};

enum class Type : unsigned char {
    Unknown = 0,
    Root    = 1,
    Mirror  = 2,
    Raidz   = 3,
    Disk    = 4,
    File    = 5,
    Hole    = 6,
    Spare   = 7,
    Log     = 8,
};

[[nodiscard]] inline bool is_faulted(State s) noexcept {
    return s == State::Faulted || s == State::Offline;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    if (static_cast<unsigned char>(s) > static_cast<unsigned char>(State::Healthy)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::vdev
''',
    "pbsd/zfs/pbsd.zfs.arc.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.arc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/arc.h — ARC buf flags.
export namespace pbsd::zfs::arc {

enum class BufFlag : unsigned int {
    Shared     = 1u << 0,
    Compressed = 1u << 1,
    Encrypted  = 1u << 2,
};

enum class BufContents : unsigned char {
    Data     = 0,
    Metadata = 1,
};

enum class SpaceType : unsigned char {
    Data   = 0,
    Meta   = 1,
    Hdrs   = 2,
    L2hdrs = 3,
    Dbuf   = 4,
    Dnode  = 5,
    Bonus  = 6,
};

[[nodiscard]] inline bool is_encrypted(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(BufFlag::Encrypted)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(BufFlag::Compressed))
        && (flags & static_cast<unsigned>(BufFlag::Encrypted)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::arc
''',
    "pbsd/zfs/pbsd.zfs.dsl.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.dsl;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dsl_dataset.h — hold flags.
export namespace pbsd::zfs::dsl {

enum class HoldFlag : unsigned int {
    None      = 0,
    Recursive = 0x01,
    Clone     = 0x02,
    Temp      = 0x04,
};

enum class PropSource : unsigned char {
    None     = 0,
    Default  = 1,
    Local    = 2,
    Inherited = 3,
    Received = 4,
};

[[nodiscard]] inline Status validate_hold(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(HoldFlag::Clone))
        && (flags & static_cast<unsigned>(HoldFlag::Temp))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl
''',
    "pbsd/stand/pbsd.stand.gpt.cppm": '''module;
#include <cstdint>

export module pbsd.stand.gpt;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/lib/gpt.h — GPT partition type GUIDs (subset).
export namespace pbsd::stand::gpt {

inline constexpr unsigned kHeaderSize = 92;
inline constexpr unsigned kEntrySize  = 128;
inline constexpr unsigned kSignature  = 0x54524150; // "PART"

enum class EntryType : unsigned int {
    EfiSystem   = 0,
    EfiBoot     = 1,
    BiosBoot    = 2,
    Freebsd     = 3,
    FreebsdBoot = 4,
    FreebsdSwap = 5,
    FreebsdZfs  = 6,
    LinuxData   = 7,
    LinuxSwap   = 8,
};

struct Header {
    unsigned signature{};
    unsigned revision{};
    unsigned header_size{};
    unsigned header_crc32{};
    unsigned long long current_lba{};
    unsigned long long backup_lba{};
    unsigned long long first_usable_lba{};
    unsigned long long last_usable_lba{};
};

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.signature != kSignature) {
        return Status::Invalid;
    }
    if (hdr.header_size < kHeaderSize) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::gpt
''',
    "pbsd/stand/pbsd.stand.kmod.cppm": '''module;
#include <cstdint>

export module pbsd.stand.kmod;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/module.h — kmod loader flags.
export namespace pbsd::stand::kmod {

enum class Flag : unsigned int {
    Glob    = 0x01,
    NoUnload = 0x02,
    Loader  = 0x04,
};

enum class Event : unsigned char {
    Load   = 0,
    Unload = 1,
    Shutdown = 2,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::NoUnload))
        && (flags & static_cast<unsigned>(Flag::Loader)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::kmod
''',
    "pbsd/stand/pbsd.stand.elf.cppm": '''module;
#include <cstdint>

export module pbsd.stand.elf;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/efi/include/elf.h — ELF class/type constants.
export namespace pbsd::stand::elf {

enum class Class : unsigned char {
    None = 0,
    Elf32 = 1,
    Elf64 = 2,
};

enum class Data : unsigned char {
    None = 0,
    Lsb  = 1,
    Msb  = 2,
};

enum class Type : unsigned short {
    None   = 0,
    Rel    = 1,
    Exec   = 2,
    Dyn    = 3,
    Core   = 4,
};

enum class Machine : unsigned short {
    None   = 0,
    Amd64  = 62,
    Arm64  = 183,
};

[[nodiscard]] inline Status validate_ehdr(Class c, Machine m) noexcept {
    if (c == Class::Elf64 && m != Machine::Amd64 && m != Machine::Arm64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::elf
''',
    "pbsd/arch/amd64/pbsd.arch.amd64.pmap.cppm": '''module;
#include <cstdint>

export module pbsd.arch.amd64.pmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/pmap.h — page table entry bits.
export namespace pbsd::arch::amd64::pmap {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned long long kPageSize = 1ull << kPageShift;

enum class PteFlag : unsigned long long {
    Present  = 1ull << 0,
    Write    = 1ull << 1,
    User     = 1ull << 2,
    Pwt      = 1ull << 3,
    Pcd      = 1ull << 4,
    Accessed = 1ull << 5,
    Dirty    = 1ull << 6,
    Large    = 1ull << 7,
    Global   = 1ull << 8,
    Nx       = 1ull << 63,
};

[[nodiscard]] inline Status validate_pte(unsigned long long pte) noexcept {
    if ((pte & static_cast<unsigned long long>(PteFlag::Present)) == 0) {
        return Status::Ok;
    }
    if ((pte & static_cast<unsigned long long>(PteFlag::Large))
        && (pte & static_cast<unsigned long long>(PteFlag::Dirty))) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::pmap
''',
    "pbsd/arch/amd64/pbsd.arch.amd64.apic.cppm": '''module;
#include <cstdint>

export module pbsd.arch.amd64.apic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/apicvar.h — APIC delivery modes.
export namespace pbsd::arch::amd64::apic {

enum class DeliveryMode : unsigned char {
    Fixed     = 0,
    Lowest    = 1,
    Smi       = 2,
    Nmi       = 4,
    Init      = 5,
    Startup   = 6,
    ExtInt    = 7,
};

enum class TriggerMode : unsigned char {
    Edge  = 0,
    Level = 1,
};

enum class DestinationMode : unsigned char {
    Physical = 0,
    Logical  = 1,
};

[[nodiscard]] inline Status validate_delivery(DeliveryMode m) noexcept {
    if (static_cast<unsigned char>(m) > static_cast<unsigned char>(DeliveryMode::ExtInt)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::apic
''',
    "pbsd/arch/arm64/pbsd.arch.arm64.pmap.cppm": '''module;
#include <cstdint>

export module pbsd.arch.arm64.pmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/pmap.h — arm64 PTE attributes.
export namespace pbsd::arch::arm64::pmap {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned long long kPageSize = 1ull << kPageShift;

enum class Attr : unsigned long long {
    Valid   = 1ull << 0,
    Table   = 1ull << 1,
    Af      = 1ull << 10,
    Ng      = 1ull << 11,
    Uxn     = 1ull << 54,
    Pxn     = 1ull << 53,
    Cont    = 1ull << 52,
};

enum class Shareability : unsigned char {
    Non     = 0,
    Outer   = 2,
    Inner   = 3,
};

[[nodiscard]] inline Status validate_pte(unsigned long long pte) noexcept {
    if ((pte & static_cast<unsigned long long>(Attr::Valid)) == 0) {
        return Status::Ok;
    }
    if ((pte & static_cast<unsigned long long>(Attr::Uxn))
        && (pte & static_cast<unsigned long long>(Attr::Pxn))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::pmap
''',
    "pbsd/arch/arm64/pbsd.arch.arm64.gic.cppm": '''module;
#include <cstdint>

export module pbsd.arch.arm64.gic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/gic_v3.h — GICv3 IRQ types.
export namespace pbsd::arch::arm64::gic {

enum class IrqType : unsigned char {
    Spurious = 0,
    Ppi      = 1,
    Spi      = 2,
    Lpi      = 3,
};

enum class Trigger : unsigned char {
    Edge  = 0,
    Level = 1,
};

inline constexpr unsigned kMaxSpi = 1019;
inline constexpr unsigned kPpiBase = 16;

[[nodiscard]] inline Status validate_irq(unsigned irq, IrqType t) noexcept {
    if (t == IrqType::Spi && irq > kMaxSpi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::gic
''',
    "pbsd/bifrost/pbsd.bifrost.vmcs.cppm": '''module;
#include <cstdint>

export module pbsd.bifrost.vmcs;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmcs.h — VMCS field encodings.
export namespace pbsd::bifrost::vmcs {

inline constexpr unsigned long long kInitial = 0xffffffffffffffffull;
inline constexpr unsigned kInvalidEncoding = 0xffffffffu;

enum class Field : unsigned int {
    Vpid              = 0x00000000,
    GuestEsSelector   = 0x00000800,
    GuestCsSelector   = 0x00000802,
    GuestRip          = 0x0000681e,
    GuestRsp          = 0x0000681c,
    ExitReason        = 0x00004402,
    ExitQualification = 0x00006400,
    InstructionError  = 0x00004400,
};

[[nodiscard]] inline unsigned ident(Field f) noexcept {
    return static_cast<unsigned>(f) | 0x80000000u;
}

[[nodiscard]] inline hypervisor::Backend backend_for_vmcs() noexcept {
    return hypervisor::Backend::Vmx;
}

[[nodiscard]] inline Status validate_field(Field f) noexcept {
    if (static_cast<unsigned>(f) == kInvalidEncoding) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vmcs
''',
    "pbsd/bifrost/pbsd.bifrost.npt.cppm": '''module;
#include <cstdint>

export module pbsd.bifrost.npt;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/amd/npt.h — AMD NPT fault bits.
export namespace pbsd::bifrost::npt {

enum class FaultFlag : unsigned long long {
    Present  = 1ull << 0,
    Write    = 1ull << 1,
    User     = 1ull << 2,
    Fetch    = 1ull << 4,
    Final    = 1ull << 32,
};

struct Fault {
    unsigned long long gpa{};
    unsigned long long flags{};
};

[[nodiscard]] inline bool is_write(Fault const& f) noexcept {
    return (f.flags & static_cast<unsigned long long>(FaultFlag::Write)) != 0;
}

[[nodiscard]] inline Status validate_fault(Fault const& f) noexcept {
    if (f.gpa == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for_npt() noexcept {
    return hypervisor::Backend::Svm;
}

} // namespace pbsd::bifrost::npt
''',
    "pbsd/compositor/pbsd.compositor.wayland.output.cppm": '''module;
#include <cstdint>

export module pbsd.compositor.wayland.output;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_output — mode/transform/subpixel enums.
export namespace pbsd::compositor::wayland::output {

enum class Subpixel : unsigned int {
    Unknown       = 0,
    None          = 1,
    HorizontalRgb = 2,
    HorizontalBgr = 3,
    VerticalRgb   = 4,
    VerticalBgr   = 5,
};

enum class Transform : unsigned int {
    Normal     = 0,
    Rotate90   = 1,
    Rotate180  = 2,
    Rotate270  = 3,
    Flipped    = 4,
    Flipped90  = 5,
    Flipped180 = 6,
    Flipped270 = 7,
};

enum class ModeFlag : unsigned int {
    Current = 0x1,
    Preferred = 0x2,
};

struct Mode {
    unsigned width{};
    unsigned height{};
    unsigned refresh_mhz{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_mode(Mode const& m) noexcept {
    if (m.width == 0 || m.height == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::output
''',
    "pbsd/compositor/pbsd.compositor.wayland.buffer.cppm": '''module;
#include <cstdint>

export module pbsd.compositor.wayland.buffer;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_buffer / wl_shm — release and format opcodes.
export namespace pbsd::compositor::wayland::buffer {

enum class ShmFormat : unsigned int {
    CArgb8888 = 0,
    CXrgb8888 = 1,
    Crgb565   = 2,
};

enum class BufferRequest : unsigned int {
    Destroy  = 0,
    Release  = 0,
};

enum class ShmPoolRequest : unsigned int {
    CreateBuffer = 0,
    Destroy      = 1,
    Resize       = 2,
};

[[nodiscard]] inline Status validate_format(ShmFormat fmt) noexcept {
    if (static_cast<unsigned>(fmt) > static_cast<unsigned>(ShmFormat::Crgb565)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bytes_per_pixel(ShmFormat fmt) noexcept {
    switch (fmt) {
    case ShmFormat::CArgb8888:
    case ShmFormat::CXrgb8888:
        return 4;
    case ShmFormat::Crgb565:
        return 2;
    default:
        return 0;
    }
}

} // namespace pbsd::compositor::wayland::buffer
''',
    "pbsd/pkg/pbsd.pkg.manifest.cppm": '''module;
#include <cstdint>

export module pbsd.pkg.manifest;

import pbsd.core;

/// Wave 9 — package manifest schema (content-addressed).
export namespace pbsd::pkg::manifest {

enum class EntryType : unsigned char {
    File      = 0,
    Directory = 1,
    Symlink   = 2,
    Device    = 3,
};

struct Entry {
    EntryType type{EntryType::File};
    char path[256]{};
    char digest_hex[65]{};
    unsigned long long size{};
};

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    if (e.path[0] == '\0') {
        return Status::Invalid;
    }
    if (e.type == EntryType::File && e.size == 0 && e.digest_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::manifest
''',
    "pbsd/pkg/pbsd.pkg.repo.cppm": '''module;
#include <cstdint>

export module pbsd.pkg.repo;

import pbsd.core;

/// Wave 9 — package repository metadata.
export namespace pbsd::pkg::repo {

enum class RepoFlag : unsigned int {
    Signed    = 0x01,
    Mirror    = 0x02,
    Local     = 0x04,
    Immutable = 0x08,
};

struct RepoRef {
    char name[64]{};
    char url[256]{};
    char fingerprint_hex[65]{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_repo(RepoRef const& r) noexcept {
    if (r.name[0] == '\0' || r.url[0] == '\0') {
        return Status::Invalid;
    }
    if ((r.flags & static_cast<unsigned>(RepoFlag::Signed))
        && r.fingerprint_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::pkg::repo
''',
    "pbsd/pkg/pbsd.pkg.signature.cppm": '''module;
#include <cstdint>

export module pbsd.pkg.signature;

import pbsd.core;

/// Wave 9 — package signature verification stubs.
export namespace pbsd::pkg::signature {

enum class Algo : unsigned char {
    None   = 0,
    Sha256 = 1,
    Ed25519 = 2,
};

struct SignatureBlock {
    Algo algo{Algo::None};
    char key_id[32]{};
    char sig_hex[129]{};
};

[[nodiscard]] inline Status validate_block(SignatureBlock const& b) noexcept {
    if (b.algo == Algo::None) {
        return Status::Invalid;
    }
    if (b.sig_hex[0] == '\0') {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status verify_digest(char const* digest_hex,
                                          SignatureBlock const& b) noexcept {
    if (digest_hex == nullptr || digest_hex[0] == '\0') {
        return Status::Invalid;
    }
    return validate_block(b);
}

} // namespace pbsd::pkg::signature
''',
}


def main() -> int:
    created = 0
    for rel, body in MODULES.items():
        path = ROOT / rel
        if path.exists():
            continue
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8", newline="\n")
        created += 1
        print(f"created {rel}")
    print(f"done: {created} new, {len(MODULES) - created} existed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
