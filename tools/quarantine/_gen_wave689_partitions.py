#!/usr/bin/env python3
"""Generate Wave 6–9 partition .cppm modules from HBSD header constants."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

MODULES: dict[str, str] = {
    "pbsd/net/pbsd.net.arp.cppm": '''module;
#include <cstdint>

export module pbsd.net.arp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_arp.h — ARP hardware/opcodes.
export namespace pbsd::net::arp {

enum class Hardware : unsigned short {
    Ether       = 1,
    Ieee802     = 6,
    FrameRelay  = 15,
    Ieee1394    = 24,
    Infiniband  = 32,
};

enum class Op : unsigned short {
    Request    = 1,
    Reply      = 2,
    RevRequest = 3,
    RevReply   = 4,
    InvRequest = 8,
    InvReply   = 9,
};

struct Header {
    unsigned short hw_type{};
    unsigned short proto_type{};
    unsigned char  hw_len{};
    unsigned char  proto_len{};
    unsigned short op{};
};

struct OpEntry {
    Op          op{};
    const char* label{};
    bool        is_request{};
};

inline constexpr OpEntry kOpTable[] = {
    {Op::Request, "request", true},
    {Op::Reply, "reply", false},
    {Op::RevRequest, "revrequest", true},
    {Op::RevReply, "revreply", false},
    {Op::InvRequest, "invrequest", true},
    {Op::InvReply, "invreply", false},
};

[[nodiscard]] inline constexpr std::size_t op_table_size() noexcept {
    return sizeof(kOpTable) / sizeof(kOpTable[0]);
}

[[nodiscard]] inline Status validate_header(Header const& hdr) noexcept {
    if (hdr.hw_len == 0 || hdr.proto_len == 0) {
        return Status::Protocol;
    }
    if (hdr.hw_type != static_cast<unsigned short>(Hardware::Ether)
        && hdr.hw_type != static_cast<unsigned short>(Hardware::Infiniband)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::arp
''',
    "pbsd/net/pbsd.net.ether.cppm": '''module;
#include <cstdint>

export module pbsd.net.ether;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/if_ether.h, net/ethernet.h — Ethernet constants.
export namespace pbsd::net::ether {

inline constexpr unsigned kAddrLen = 6;
inline constexpr unsigned kTypeIp  = 0x0800;
inline constexpr unsigned kTypeIpv6 = 0x86DD;
inline constexpr unsigned kTypeArp = 0x0806;

enum class Type : unsigned short {
    Ip   = kTypeIp,
    Ipv6 = kTypeIpv6,
    Arp  = kTypeArp,
};

struct Address {
    unsigned char octets[kAddrLen]{};
};

[[nodiscard]] inline bool is_broadcast(Address const& a) noexcept {
    for (unsigned i = 0; i < kAddrLen; ++i) {
        if (a.octets[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_multicast(Address const& a) noexcept {
    return (a.octets[0] & 0x01) != 0;
}

[[nodiscard]] inline Status validate_type(unsigned short t) noexcept {
    if (t != kTypeIp && t != kTypeIpv6 && t != kTypeArp) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ether
''',
    "pbsd/fs/pbsd.fs.stat.cppm": '''module;
#include <cstdint>

export module pbsd.fs.stat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/stat.h — S_IF* file type bits and UF_* flags.
export namespace pbsd::fs::stat {

inline constexpr unsigned kBlksize = 512;

enum class FileType : unsigned int {
    Fifo    = 0x1000,
    Char    = 0x2000,
    Dir     = 0x4000,
    Block   = 0x6000,
    Regular = 0x8000,
    Link    = 0xA000,
    Socket  = 0xC000,
    Whiteout = 0xE000,
};

enum class UfFlag : unsigned int {
    Nodump   = 0x00000001,
    Immutable = 0x00000002,
    Append   = 0x00000004,
    Opaque   = 0x00000008,
    Nounlink = 0x00000010,
    Hidden   = 0x00008000,
    Archive  = 0x00010000,
};

[[nodiscard]] inline constexpr FileType mode_to_type(unsigned mode) noexcept {
    return static_cast<FileType>(mode & 0xF000);
}

[[nodiscard]] inline constexpr bool is_dir(unsigned mode) noexcept {
    return mode_to_type(mode) == FileType::Dir;
}

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    switch (mode_to_type(mode)) {
    case FileType::Fifo:
    case FileType::Char:
    case FileType::Dir:
    case FileType::Block:
    case FileType::Regular:
    case FileType::Link:
    case FileType::Socket:
    case FileType::Whiteout:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::stat
''',
    "pbsd/fs/pbsd.fs.fcntl.cppm": '''module;
#include <cstdint>

export module pbsd.fs.fcntl;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/fcntl.h — O_* open flags and F_* fcntl commands.
export namespace pbsd::fs::fcntl {

enum class OpenFlag : unsigned int {
    Rdonly   = 0x0000,
    Wronly   = 0x0001,
    Rdwr     = 0x0002,
    Nonblock = 0x0004,
    Append   = 0x0008,
    Shlock   = 0x0010,
    Exlock   = 0x0020,
    Async    = 0x0040,
    Sync     = 0x0080,
    NoFollow = 0x0100,
    Creat    = 0x0200,
    Trunc    = 0x0400,
    Excl     = 0x0800,
    Direct   = 0x00010000,
    Directory = 0x00020000,
};

enum class FcntlCmd : unsigned int {
    DupFd       = 0,
    GetFd       = 1,
    SetFd       = 2,
    GetFlags    = 3,
    SetFlags    = 4,
    GetOwn      = 5,
    SetOwn      = 6,
    GetLk       = 7,
    SetLk       = 8,
    SetLkw      = 9,
    GetOwnEx    = 16,
    SetOwnEx    = 17,
};

[[nodiscard]] inline CapabilityRights rights_for(OpenFlag f) noexcept {
    switch (f) {
    case OpenFlag::Rdonly:
        return CapabilityRights::Read;
    case OpenFlag::Wronly:
    case OpenFlag::Append:
    case OpenFlag::Trunc:
    case OpenFlag::Creat:
        return CapabilityRights::Write;
    case OpenFlag::Rdwr:
        return CapabilityRights::Read | CapabilityRights::Write;
    default:
        return CapabilityRights::None;
    }
}

[[nodiscard]] inline Status check_open(CapabilityRights held, OpenFlag f) noexcept {
    CapabilityRights need = rights_for(f);
    return has_right(held, need) ? Status::Ok : Status::AccessDenied;
}

} // namespace pbsd::fs::fcntl
''',
    "pbsd/fs/pbsd.fs.dirent.cppm": '''module;
#include <cstdint>

export module pbsd.fs.dirent;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/dirent.h — DT_* dirent type constants.
export namespace pbsd::fs::dirent {

inline constexpr unsigned kMaxNameLen = 255;

enum class Type : unsigned char {
    Unknown = 0,
    Fifo    = 1,
    Chr     = 2,
    Dir     = 4,
    Blk     = 6,
    Reg     = 8,
    Lnk     = 10,
    Sock    = 12,
    Wht     = 14,
};

struct Entry {
    unsigned long long ino{};
    unsigned char      type{};
    unsigned short     reclen{};
    unsigned char      namelen{};
};

[[nodiscard]] inline constexpr bool is_dir(Type t) noexcept {
    return t == Type::Dir;
}

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    if (e.namelen > kMaxNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::dirent
''',
    "pbsd/geom/pbsd.geom.disk.cppm": '''module;
#include <cstdint>

export module pbsd.geom.disk;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_disk.h — DISK rotation-rate constants.
export namespace pbsd::geom::disk {

inline constexpr unsigned kClassName = 0x4449534B; // "DISK"

enum class RotationRate : unsigned int {
    Unknown      = 0,
    NonRotating  = 1,
    Min          = 0x0401,
    Max          = 0xFFFE,
};

enum class InitLevel : unsigned char {
    None   = 0,
    Create = 1,
    Start  = 2,
    Done   = 3,
};

[[nodiscard]] inline Status validate_rotation_rate(unsigned rate) noexcept {
    if (rate == static_cast<unsigned>(RotationRate::Unknown)
        || rate == static_cast<unsigned>(RotationRate::NonRotating)) {
        return Status::Ok;
    }
    if (rate >= static_cast<unsigned>(RotationRate::Min)
        && rate <= static_cast<unsigned>(RotationRate::Max)) {
        return Status::Ok;
    }
    return Status::Invalid;
}

} // namespace pbsd::geom::disk
''',
    "pbsd/geom/pbsd.geom.slice.cppm": '''module;
#include <cstdint>

export module pbsd.geom.slice;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_slice.h — GEOM slice geometry.
export namespace pbsd::geom::slice {

struct Slice {
    long long offset{};
    long long length{};
    unsigned  sector_size{};
};

struct HotSlice {
    long long offset{};
    long long length{};
    int       read_active{};
    int       delete_active{};
    int       write_active{};
};

[[nodiscard]] inline Status validate_slice(Slice const& s) noexcept {
    if (s.length <= 0 || s.sector_size == 0) {
        return Status::Invalid;
    }
    if (s.offset < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline long long end_offset(Slice const& s) noexcept {
    return s.offset + s.length;
}

} // namespace pbsd::geom::slice
''',
    "pbsd/geom/pbsd.geom.flashmap.cppm": '''module;
#include <cstdint>

export module pbsd.geom.flashmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_flashmap.h — flashmap GEOM class.
export namespace pbsd::geom::flashmap {

inline constexpr unsigned kClassName = 0x464C5348; // "FLSH"

enum class Flag : unsigned int {
    ReadOnly  = 0x01,
    WriteOnly = 0x02,
    Erase     = 0x04,
};

struct Region {
    unsigned long long offset{};
    unsigned long long length{};
    unsigned           erase_size{};
};

[[nodiscard]] inline Status validate_region(Region const& r) noexcept {
    if (r.length == 0 || r.erase_size == 0) {
        return Status::Invalid;
    }
    if (r.offset % r.erase_size != 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::flashmap
''',
    "pbsd/zfs/pbsd.zfs.props.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.props;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/zfs_prop.h — zprop types/attrs.
export namespace pbsd::zfs::props {

enum class PropType : unsigned char {
    Number = 0,
    String = 1,
    Index  = 2,
};

enum class PropAttr : unsigned char {
    Default         = 0,
    Readonly        = 1,
    Inherit         = 2,
    Onetime         = 3,
    OnetimeDefault  = 4,
};

enum class DatasetProp : unsigned int {
    Type        = 0,
    Creation    = 1,
    Used        = 2,
    Available   = 3,
    Referenced  = 4,
    Compress    = 5,
    Checksum    = 6,
    Recordsize  = 7,
    Mountpoint  = 8,
    Quota       = 9,
    Encryption  = 10,
};

struct PropEntry {
    DatasetProp prop{};
    PropType    type{};
    PropAttr    attr{};
    const char* name{};
};

inline constexpr PropEntry kPropTable[] = {
    {DatasetProp::Type, PropType::String, PropAttr::Readonly, "type"},
    {DatasetProp::Creation, PropType::Number, PropAttr::Readonly, "creation"},
    {DatasetProp::Used, PropType::Number, PropAttr::Readonly, "used"},
    {DatasetProp::Available, PropType::Number, PropAttr::Readonly, "available"},
    {DatasetProp::Compress, PropType::Index, PropAttr::Inherit, "compression"},
    {DatasetProp::Checksum, PropType::Index, PropAttr::Inherit, "checksum"},
    {DatasetProp::Recordsize, PropType::Number, PropAttr::Inherit, "recordsize"},
    {DatasetProp::Mountpoint, PropType::String, PropAttr::Inherit, "mountpoint"},
    {DatasetProp::Quota, PropType::Number, PropAttr::Default, "quota"},
    {DatasetProp::Encryption, PropType::Index, PropAttr::Onetime, "encryption"},
};

[[nodiscard]] inline constexpr std::size_t prop_table_size() noexcept {
    return sizeof(kPropTable) / sizeof(kPropTable[0]);
}

[[nodiscard]] inline Status validate_attr(PropAttr a) noexcept {
    if (static_cast<unsigned char>(a) > static_cast<unsigned char>(PropAttr::OnetimeDefault)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::props
''',
    "pbsd/zfs/pbsd.zfs.zio.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.zio;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO priority/flags.
export namespace pbsd::zfs::zio {

enum class Priority : unsigned char {
    Now      = 0,
    Sync     = 1,
    Async    = 2,
    Scrub    = 3,
    Idle     = 4,
};

enum class Flag : unsigned int {
    Read        = 0x00000001,
    Write       = 0x00000002,
    Free        = 0x00000004,
    Allocate    = 0x00000008,
    Defer       = 0x00000010,
    Nodata      = 0x00000020,
    SetError    = 0x00000040,
    Config      = 0x00000080,
    Rewrite     = 0x00000100,
    Raw         = 0x00000200,
};

enum class Stage : unsigned char {
    Open     = 0,
    Read     = 1,
    Write    = 2,
    Free     = 3,
    Done     = 4,
    Error    = 5,
};

[[nodiscard]] inline constexpr bool is_read(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Read)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::Read))
        && (flags & static_cast<unsigned>(Flag::Write))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::zio
''',
    "pbsd/zfs/pbsd.zfs.spa.cppm": '''module;
#include <cstdint>

export module pbsd.zfs.spa;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/spa.h — SPA state/import flags.
export namespace pbsd::zfs::spa {

enum class State : unsigned char {
    Active   = 0,
    Exporting = 1,
    Exported = 2,
    Destroyed = 3,
};

enum class ImportFlag : unsigned int {
    Normal          = 0x0,
    Verbose         = 0x1,
    MissingLog      = 0x2,
    OnlyMissingLog  = 0x4,
    TempName        = 0x8,
    Bootable        = 0x10,
    AltRoot         = 0x20,
    ForceSpare      = 0x40,
};

enum class LoadPolicy : unsigned char {
    Cache  = 0,
    Memory = 1,
    None   = 2,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    if (static_cast<unsigned char>(s) > static_cast<unsigned char>(State::Destroyed)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_importable(State s) noexcept {
    return s == State::Exported || s == State::Active;
}

} // namespace pbsd::zfs::spa
''',
    "pbsd/stand/pbsd.stand.loader.cppm": '''module;
#include <cstdint>

export module pbsd.stand.loader;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/commands.c, boot.c — loader command opcodes.
export namespace pbsd::stand::loader {

enum class Cmd : unsigned int {
    Boot     = 0,
    Load     = 1,
    Unload   = 2,
    Ls       = 3,
    Cat      = 4,
    More     = 5,
    Include  = 6,
    Help     = 7,
    Reboot   = 8,
    Pwd      = 9,
    Set      = 10,
    Echo     = 11,
    Read     = 12,
    Bcopy    = 13,
    Malloc   = 14,
    Free     = 15,
};

struct CmdEntry {
    Cmd         cmd{};
    const char* name{};
    bool        needs_arg{};
};

inline constexpr CmdEntry kCmdTable[] = {
    {Cmd::Boot, "boot", false},
    {Cmd::Load, "load", true},
    {Cmd::Unload, "unload", true},
    {Cmd::Ls, "ls", true},
    {Cmd::Cat, "cat", true},
    {Cmd::Help, "help", false},
    {Cmd::Reboot, "reboot", false},
    {Cmd::Set, "set", true},
    {Cmd::Echo, "echo", true},
};

[[nodiscard]] inline constexpr std::size_t cmd_table_size() noexcept {
    return sizeof(kCmdTable) / sizeof(kCmdTable[0]);
}

[[nodiscard]] inline Status validate_cmd(Cmd c) noexcept {
    if (static_cast<unsigned>(c) > static_cast<unsigned>(Cmd::Free)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::loader
''',
    "pbsd/stand/pbsd.stand.reboot.cppm": '''module;
#include <cstdint>

export module pbsd.stand.reboot;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/reboot.h — RB_* reboot flags.
export namespace pbsd::stand::reboot {

enum class Flag : unsigned int {
    AutoBoot   = 0x0001,
    AskName    = 0x0002,
    Single     = 0x0004,
    Dump       = 0x0100,
    InitName   = 0x0200,
    PowerOff   = 0x0400,
    PowerCycle = 0x0800,
    BootInfo   = 0x1000,
    Kdb        = 0x2000,
    Color      = 0x4000,
    Pause      = 0x8000,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::PowerOff))
        && (flags & static_cast<unsigned>(Flag::PowerCycle))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool wants_dump(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(Flag::Dump)) != 0;
}

} // namespace pbsd::stand::reboot
''',
    "pbsd/arch/amd64/pbsd.arch.amd64.cpufunc.cppm": '''module;
#include <cstdint>

export module pbsd.arch.amd64.cpufunc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/cpufunc.h — x86 control-register bits.
export namespace pbsd::arch::amd64::cpufunc {

enum class Cr0Flag : unsigned long long {
    Pe  = 1ull << 0,
    Mp  = 1ull << 1,
    Em  = 1ull << 2,
    Ts  = 1ull << 3,
    Et  = 1ull << 4,
    Ne  = 1ull << 5,
    Wp  = 1ull << 16,
    Am  = 1ull << 18,
    Nw  = 1ull << 29,
    Cd  = 1ull << 30,
    Pg  = 1ull << 31,
};

enum class Cr4Flag : unsigned long long {
    Vme = 1ull << 0,
    Pvi = 1ull << 1,
    Tsd = 1ull << 2,
    De  = 1ull << 3,
    Pse = 1ull << 4,
    Pae = 1ull << 5,
    Mce = 1ull << 6,
    Pge = 1ull << 7,
    Pce = 1ull << 8,
    Osfxsr = 1ull << 9,
    Osxmmexcpt = 1ull << 10,
    Umip = 1ull << 11,
    La57 = 1ull << 12,
    VmxE = 1ull << 13,
    SmxE = 1ull << 14,
    Fsgsbase = 1ull << 16,
    Pcid = 1ull << 17,
    Osxsave = 1ull << 18,
    Smep = 1ull << 20,
    Smap = 1ull << 21,
};

[[nodiscard]] inline Status validate_cr0(unsigned long long val) noexcept {
    if ((val & static_cast<unsigned long long>(Cr0Flag::Pg))
        && !(val & static_cast<unsigned long long>(Cr0Flag::Pe))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cr4_vmx(unsigned long long val) noexcept {
    if ((val & static_cast<unsigned long long>(Cr4Flag::VmxE))
        && !(val & static_cast<unsigned long long>(Cr4Flag::Pae))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::cpufunc
''',
    "pbsd/arch/arm64/pbsd.arch.arm64.exception.cppm": '''module;
#include <cstdint>

export module pbsd.arch.arm64.exception;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/exception.h — ESR_ELx exception classes.
export namespace pbsd::arch::arm64::exception {

enum class Ec : unsigned char {
    Unknown         = 0x00,
    Wfi             = 0x01,
    MrcMcrCp15      = 0x03,
    MrcMcrCp14      = 0x04,
    LdcStc          = 0x05,
    SimdFp          = 0x07,
    MrcMcrCp10      = 0x08,
    BranchTarget    = 0x0A,
    IllegalState    = 0x0E,
    Svc32           = 0x11,
    Svc64           = 0x15,
    InstAbtLower    = 0x20,
    InstAbtCurrent  = 0x21,
    DataAbtLower    = 0x24,
    DataAbtCurrent  = 0x25,
    IeAbtLower      = 0x28,
    IeAbtCurrent    = 0x29,
    SError          = 0x2F,
};

[[nodiscard]] inline unsigned ec_from_esr(unsigned long long esr) noexcept {
    return static_cast<unsigned>((esr >> 26) & 0x3F);
}

[[nodiscard]] inline Status validate_ec(Ec ec) noexcept {
    if (static_cast<unsigned char>(ec) > static_cast<unsigned char>(Ec::SError)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_sync_abort(Ec ec) noexcept {
    return ec == Ec::InstAbtLower || ec == Ec::InstAbtCurrent
        || ec == Ec::DataAbtLower || ec == Ec::DataAbtCurrent;
}

} // namespace pbsd::arch::arm64::exception
''',
    "pbsd/bifrost/pbsd.bifrost.vmcb.cppm": '''module;
#include <cstdint>

export module pbsd.bifrost.vmcb;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/amd/vmcb.h — VMCB intercept bits.
export namespace pbsd::bifrost::vmcb {

inline constexpr unsigned kCrIntcpt  = 0;
inline constexpr unsigned kDrIntcpt  = 1;
inline constexpr unsigned kExcIntcpt = 2;
inline constexpr unsigned kCtrl1Intcpt = 3;
inline constexpr unsigned kCtrl2Intcpt = 4;

enum class Ctrl1 : unsigned int {
    Intr        = 1u << 0,
    Nmi         = 1u << 1,
    Smi         = 1u << 2,
    Init        = 1u << 3,
    Vintr       = 1u << 4,
    Cr0Write    = 1u << 5,
    Rdtsc       = 1u << 14,
    Rdpmc       = 1u << 15,
    Cpuid       = 1u << 18,
    Hlt         = 1u << 24,
    Invlpg      = 1u << 25,
    Io          = 1u << 27,
    Msr         = 1u << 28,
    TaskSwitch  = 1u << 29,
};

enum class Ctrl2 : unsigned int {
    Vmrun  = 1u << 0,
    Vmmcall = 1u << 1,
    Vmload = 1u << 2,
    Vmsave = 1u << 3,
    Stgi   = 1u << 4,
    Clgi   = 1u << 5,
    Skinit = 1u << 6,
    Rdtscp = 1u << 7,
    Icebp  = 1u << 8,
    Wbinvd = 1u << 9,
    Monitor = 1u << 10,
    Mwait  = 1u << 11,
    Xsetbv = 1u << 13,
    Rdpru  = 1u << 14,
    EferWrite = 1u << 15,
    CrWrite = 1u << 16,
    CrRead  = 1u << 17,
    Invlpga = 1u << 18,
    Iopm    = 1u << 19,
    Msrpm   = 1u << 20,
    TaskSwitch = 1u << 21,
    FerrFreeze = 1u << 22,
    Shutdown   = 1u << 23,
};

[[nodiscard]] inline Status validate_intercept(unsigned ctrl1, unsigned ctrl2) noexcept {
    if ((ctrl1 & static_cast<unsigned>(Ctrl1::Cpuid))
        && (ctrl2 & static_cast<unsigned>(Ctrl2::Vmrun))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for_vmcb() noexcept {
    return hypervisor::Backend::Svm;
}

} // namespace pbsd::bifrost::vmcb
''',
    "pbsd/compositor/pbsd.compositor.wayland.seat.cppm": '''module;
#include <cstdint>

export module pbsd.compositor.wayland.seat;

import pbsd.core;

/// PROVENANCE: wayland.xml wl_seat / wl_pointer / wl_keyboard — event/request opcodes.
export namespace pbsd::compositor::wayland::seat {

enum class SeatEvent : unsigned int {
    Capabilities = 0,
    Name         = 1,
};

enum class PointerRequest : unsigned int {
    SetCursor    = 0,
    Release      = 1,
};

enum class PointerEvent : unsigned int {
    Enter  = 0,
    Leave  = 1,
    Motion = 2,
    Button = 3,
    Axis   = 4,
    Frame  = 5,
    AxisSource = 6,
    AxisStop   = 7,
    AxisDiscrete = 8,
    AxisValue120 = 9,
};

enum class Capability : unsigned int {
    Pointer  = 1,
    Keyboard = 2,
    Touch    = 4,
};

[[nodiscard]] inline bool has_capability(unsigned caps, Capability c) noexcept {
    return (caps & static_cast<unsigned>(c)) != 0;
}

[[nodiscard]] inline Status validate_capabilities(unsigned caps) noexcept {
    if (caps == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::seat
''',
    "pbsd/compositor/pbsd.compositor.wayland.xdg.cppm": '''module;
#include <cstdint>

export module pbsd.compositor.wayland.xdg;

import pbsd.core;

/// PROVENANCE: xdg-shell stable — toplevel/surface/positioner/decoration opcodes.
export namespace pbsd::compositor::wayland::xdg {

enum class ToplevelRequest : unsigned int {
    Destroy          = 0,
    SetParent        = 1,
    SetTitle         = 2,
    SetAppId         = 3,
    ShowWindowMenu   = 4,
    Move             = 5,
    Resize           = 6,
    SetMaxSize       = 7,
    SetMinSize       = 8,
    SetMaximized     = 9,
    UnsetMaximized   = 10,
    SetFullscreen    = 11,
    UnsetFullscreen  = 12,
    SetMinimized     = 13,
};

enum class PositionerRequest : unsigned int {
    Destroy       = 0,
    SetSize       = 1,
    SetAnchorRect = 2,
    SetAnchor     = 3,
    SetGravity    = 4,
    SetConstraintAdjustment = 5,
    SetOffset     = 6,
    SetReactive   = 7,
    SetParentSize = 8,
    SetParentConfigure = 9,
};

enum class DecorationRequest : unsigned int {
    Destroy  = 0,
    SetMode  = 1,
    UnsetMode = 2,
};

enum class WmCapability : unsigned int {
    WindowMenu     = 1,
    Maximize       = 2,
    Fullscreen     = 4,
    Minimize       = 8,
};

[[nodiscard]] inline bool is_toplevel_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ToplevelRequest::SetMinimized);
}

[[nodiscard]] inline bool is_positioner_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(PositionerRequest::SetParentConfigure);
}

[[nodiscard]] inline Status validate_wm_capabilities(unsigned caps) noexcept {
    if (caps > (static_cast<unsigned>(WmCapability::WindowMenu)
                | static_cast<unsigned>(WmCapability::Maximize)
                | static_cast<unsigned>(WmCapability::Fullscreen)
                | static_cast<unsigned>(WmCapability::Minimize))) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::xdg
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
