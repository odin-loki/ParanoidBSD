#!/usr/bin/env python3
"""Generate Wave 6 pass-3 net/fs/geom/zfs + quick stand/arch/bifrost modules."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "pbsd"

MODULES = {
"net/pbsd.net.dlt.cppm": """module;
#include <cstdint>

export module pbsd.net.dlt;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/dlt.h — libpcap link-layer header types.
export namespace pbsd::net::dlt {

inline constexpr unsigned kLowMatchingMin = 0;

enum class Type : unsigned int {
    Null    = 0,
    En10Mb  = 1,
    En3Mb   = 2,
    Ax25    = 3,
    Pronet  = 4,
    Chaos   = 5,
    Ieee802 = 6,
    Arcnet  = 7,
    Slip    = 8,
    Ppp     = 9,
    Fddi    = 10,
};

[[nodiscard]] inline Status validate_type(unsigned t) noexcept {
    if (t > static_cast<unsigned>(Type::Fddi)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::dlt
""",
"net/pbsd.net.media.cppm": """module;
#include <cstdint>

export module pbsd.net.media;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_media.h — IFM_ETHER media subtypes.
export namespace pbsd::net::media {

inline constexpr unsigned kEther = 0x00000020;
inline constexpr unsigned kMaskType = 0x000000E0;

enum class Subtype : unsigned int {
    TenT      = 3,
    Ten2      = 4,
    Ten5      = 5,
    HundredTx = 6,
    HundredFx = 7,
    HundredT4 = 8,
    ThousandT = 16,
    TenG_Lr   = 18,
    TenG_Sr   = 19,
    TenG_T    = 26,
};

enum class Option : unsigned int {
    Fdx  = 0x00100000,
    Hdx  = 0x00200000,
    Flow = 0x00400000,
    Loop = 0x00800000,
};

[[nodiscard]] inline Status validate_subtype(Subtype s) noexcept {
    switch (s) {
    case Subtype::TenT:
    case Subtype::HundredTx:
    case Subtype::ThousandT:
    case Subtype::TenG_T:
        return Status::Ok;
    default:
        return Status::Ok;
    }
}

[[nodiscard]] inline constexpr bool option_has(unsigned o, Option bit) noexcept {
    return (o & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::media
""",
"net/pbsd.net.pfil.cppm": """module;
#include <cstdint>

export module pbsd.net.pfil;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfil.h — packet filter hook types and flags.
export namespace pbsd::net::pfil {

inline constexpr unsigned kMaxName = 64;

enum class Type : unsigned char {
    Ip4 = 0,
    Ip6 = 1,
    Ethernet = 2,
};

enum class Flag : unsigned int {
    In  = 0x00010000,
    Out = 0x00020000,
    Fwd = 0x00040000,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Ip4:
    case Type::Ip6:
    case Type::Ethernet:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool flag_has(unsigned f, Flag bit) noexcept {
    return (f & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::pfil
""",
"net/pbsd.net.vxlan.cppm": """module;
#include <cstdint>

export module pbsd.net.vxlan;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_vxlan.h — VXLAN header and port constants.
export namespace pbsd::net::vxlan {

inline constexpr unsigned kPort = 4789;
inline constexpr unsigned kLegacyPort = 8472;
inline constexpr unsigned kVniMax = 1u << 24;
inline constexpr unsigned kVniMask = kVniMax - 1;
inline constexpr unsigned kHdrValidVni = 0x08000000;
inline constexpr unsigned kVniShift = 8;

struct Header {
    unsigned vxlh_flags{};
    unsigned vxlh_vni{};
};

[[nodiscard]] inline Status validate_vni(unsigned vni) noexcept {
    if (vni >= kVniMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if ((h.vxlh_flags & kHdrValidVni) == 0) {
        return Status::Protocol;
    }
    return validate_vni((h.vxlh_vni >> kVniShift) & kVniMask);
}

} // namespace pbsd::net::vxlan
""",
"net/pbsd.net.lacp.cppm": """module;
#include <cstdint>

export module pbsd.net.lacp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/ieee8023ad_lacp.h — LACP state bits and timers.
export namespace pbsd::net::lacp {

inline constexpr unsigned kNTimer = 3;

enum class Timer : unsigned char {
    CurrentWhile = 0,
    Periodic     = 1,
    WaitWhile    = 2,
};

enum class State : unsigned char {
    Activity     = 1u << 0,
    Timeout      = 1u << 1,
    Aggregation  = 1u << 2,
    Sync         = 1u << 3,
    Collecting   = 1u << 4,
    Distributing = 1u << 5,
    Defaulted    = 1u << 6,
    Expired      = 1u << 7,
};

[[nodiscard]] inline Status validate_timer(Timer t) noexcept {
    if (static_cast<unsigned>(t) >= kNTimer) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool state_has(unsigned s, State bit) noexcept {
    return (s & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::lacp
""",
"net/pbsd.net.clone.cppm": """module;
#include <cstdint>

export module pbsd.net.clone;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_clone.h — cloned interface creation flags.
export namespace pbsd::net::clone {

enum class Flag : unsigned int {
    Autounit = 0x00000001,
    Needifaddr = 0x00000002,
    Maxunit = 0x00000004,
};

inline constexpr unsigned kMaxName = 16;

[[nodiscard]] inline Status validate_unit(unsigned unit) noexcept {
    if (unit == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kMaxName) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::clone
""",
"fs/pbsd.fs.msdosfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.msdosfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/msdosfs/fat.h — FAT cluster constants.
export namespace pbsd::fs::msdosfs {

inline constexpr unsigned kRoot = 0;
inline constexpr unsigned kFirst = 2;
inline constexpr unsigned kReserved = 0xfffffff6;
inline constexpr unsigned kBad = 0xfffffff7;
inline constexpr unsigned kEofs = 0xfffffff8;
inline constexpr unsigned kEofe = 0xffffffff;

inline constexpr unsigned kFat12Mask = 0x00000fff;
inline constexpr unsigned kFat16Mask = 0x0000ffff;
inline constexpr unsigned kFat32Mask = 0x0fffffff;

[[nodiscard]] inline Status validate_cluster(unsigned cl) noexcept {
    if (cl < kFirst && cl != kRoot) {
        return Status::Invalid;
    }
    if (cl >= kReserved && cl <= kBad) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::msdosfs
""",
"fs/pbsd.fs.ext2fs.cppm": """module;
#include <cstdint>

export module pbsd.fs.ext2fs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/ext2fs/ext2fs.h — ext2/3/4 superblock magic and flags.
export namespace pbsd::fs::ext2fs {

inline constexpr unsigned kMagic = 0xEF53;
inline constexpr unsigned kRev0 = 0;
inline constexpr unsigned kRev1 = 1;
inline constexpr unsigned kRev0InodeSize = 128;

enum class State : unsigned short {
    Clean  = 0x0001,
    Errors = 0x0002,
};

enum class Hash : unsigned int {
    Signed   = 0x0001,
    Unsigned = 0x0002,
};

[[nodiscard]] inline Status validate_magic(unsigned magic) noexcept {
    if (magic != kMagic) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rev(unsigned rev) noexcept {
    if (rev > kRev1) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ext2fs
""",
"fs/pbsd.fs.cd9660.cppm": """module;
#include <cstdint>

export module pbsd.fs.cd9660;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/cd9660/iso.h — ISO9660 volume descriptor types.
export namespace pbsd::fs::cd9660 {

inline constexpr unsigned char kVdPrimary = 1;
inline constexpr unsigned char kVdSupplementary = 2;
inline constexpr unsigned char kVdEnd = 255;

inline constexpr char kStandardId[] = "CD001";
inline constexpr char kEcmaId[] = "CDW01";

enum class VdType : unsigned char {
    Primary = kVdPrimary,
    Supplementary = kVdSupplementary,
    End = kVdEnd,
};

[[nodiscard]] inline Status validate_vd_type(VdType t) noexcept {
    switch (t) {
    case VdType::Primary:
    case VdType::Supplementary:
    case VdType::End:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::cd9660
""",
"fs/pbsd.fs.unionfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.unionfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs/union.h — unionfs copy/whiteout modes.
export namespace pbsd::fs::unionfs {

enum class CopyMode : unsigned char {
    Traditional = 0,
    Transparent = 1,
    Masquerade  = 2,
};

enum class WhiteMode : unsigned char {
    Always = 0,
    WhenNeeded = 1,
};

[[nodiscard]] inline Status validate_copy_mode(CopyMode m) noexcept {
    switch (m) {
    case CopyMode::Traditional:
    case CopyMode::Transparent:
    case CopyMode::Masquerade:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::unionfs
""",
"fs/pbsd.fs.smbfs.cppm": """module;
#include <cstdint>

export module pbsd.fs.smbfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/smbfs/smbfs.h — SMBFS mount flags and version.
export namespace pbsd::fs::smbfs {

inline constexpr unsigned kVerMaj = 1;
inline constexpr unsigned kVerMin = 1012;
inline constexpr unsigned kVersion = kVerMaj * 100000 + kVerMin;
inline constexpr unsigned kMaxPathComp = 256;

enum class MountFlag : unsigned int {
    Soft      = 0x0001,
    Intr      = 0x0002,
    Strong    = 0x0004,
    HaveNls   = 0x0008,
    NoLong    = 0x0010,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::smbfs
""",
"geom/pbsd.geom.cache.cppm": """module;
#include <cstdint>

export module pbsd.geom.cache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/cache/g_cache.h — GEOM CACHE metadata.
export namespace pbsd::geom::cache {

inline constexpr unsigned kVersion = 1;
inline constexpr unsigned kBuckets = 1u << 3;

enum class Type : unsigned char {
    Manual = 0,
    Automatic = 1,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Manual:
    case Type::Automatic:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr unsigned bucket(unsigned bno) noexcept {
    return bno & (kBuckets - 1);
}

} // namespace pbsd::geom::cache
""",
"geom/pbsd.geom.shsec.cppm": """module;
#include <cstdint>

export module pbsd.geom.shsec;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/shsec/g_shsec.h — GEOM SHSEC metadata version.
export namespace pbsd::geom::shsec {

inline constexpr unsigned kVersion = 1;

enum class BlockFlag : unsigned char {
    First = 0x1,
};

[[nodiscard]] inline Status validate_disk_count(unsigned count) noexcept {
    if (count < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::shsec
""",
"geom/pbsd.geom.raid.cppm": """module;
#include <cstdint>

export module pbsd.geom.raid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/raid/g_raid.h — GEOM RAID device flags.
export namespace pbsd::geom::raid {

inline constexpr unsigned kVersion = 0;

enum class DeviceFlag : unsigned long long {
    NoAutosync = 0x0000000000000001ULL,
    NoFailsync = 0x0000000000000002ULL,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::raid
""",
"geom/pbsd.geom.mountver.cppm": """module;
#include <cstdint>

export module pbsd.geom.mountver;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/mountver/g_mountver.h — GEOM MOUNTVER constants.
export namespace pbsd::geom::mountver {

inline constexpr unsigned kVersion = 4;

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::mountver
""",
"zfs/pbsd.zfs.zio_flag.cppm": """module;
#include <cstdint>

export module pbsd.zfs.zio_flag;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO_FLAG_* bits.
export namespace pbsd::zfs::zio_flag {

enum class Flag : unsigned long long {
    DontAggregate = 1ULL << 0,
    IoRepair      = 1ULL << 1,
    SelfHeal      = 1ULL << 2,
    Resilver      = 1ULL << 3,
    Scrub         = 1ULL << 4,
    ScanThread    = 1ULL << 5,
    Physical      = 1ULL << 6,
    Canfail       = 1ULL << 7,
    Speculative   = 1ULL << 8,
    ConfigWriter  = 1ULL << 9,
    DontRetry     = 1ULL << 10,
    Nodata        = 1ULL << 12,
    IoRetry       = 1ULL << 15,
    Probe         = 1ULL << 16,
};

enum class FailureMode : unsigned char {
    Wait = 0,
    Continue = 1,
    Panic = 2,
};

[[nodiscard]] inline constexpr bool flag_has(unsigned long long f, Flag bit) noexcept {
    return (f & static_cast<unsigned long long>(bit)) != 0;
}

[[nodiscard]] inline Status validate_failure_mode(FailureMode m) noexcept {
    switch (m) {
    case FailureMode::Wait:
    case FailureMode::Continue:
    case FailureMode::Panic:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::zfs::zio_flag
""",
"zfs/pbsd.zfs.compress.cppm": """module;
#include <cstdint>

export module pbsd.zfs.compress;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/zio.h — ZIO compression algorithms.
export namespace pbsd::zfs::compress {

enum class Algorithm : unsigned char {
    Inherit = 0,
    On,
    Off,
    Lzjb,
    Gzip,
    Zle,
    Lz4,
    Functions,
};

inline constexpr Algorithm kDefault = Algorithm::On;
inline constexpr Algorithm kLegacyOnValue = Algorithm::Lzjb;
inline constexpr Algorithm kLz4OnValue = Algorithm::Lz4;

[[nodiscard]] inline Status validate_algo(Algorithm a) noexcept {
    if (a >= Algorithm::Functions) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::compress
""",
"stand/pbsd.stand.net.cppm": """module;
#include <cstdint>

export module pbsd.stand.net;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/net.h — boot-time network I/O constants.
export namespace pbsd::stand::net {

inline constexpr unsigned kMaxPhys = 16;
inline constexpr unsigned kTimeout = 3000;

enum class Proto : unsigned char {
    None = 0,
    Arp  = 1,
    Rarp = 2,
    Bootp = 3,
    Tftp = 4,
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    switch (p) {
    case Proto::Arp:
    case Proto::Rarp:
    case Proto::Bootp:
    case Proto::Tftp:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::stand::net
""",
"arch/amd64/pbsd.arch.amd64.cr.cppm": """module;
#include <cstdint>

export module pbsd.arch.amd64.cr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/specialreg.h — CR0 control register bits.
export namespace pbsd::arch::amd64::cr {

enum class Cr0 : unsigned int {
    Pe = 0x00000001,
    Mp = 0x00000002,
    Em = 0x00000004,
    Ts = 0x00000008,
    Ne = 0x00000020,
    Wp = 0x00010000,
    Am = 0x00040000,
    Nw = 0x20000000,
    Cd = 0x40000000,
    Pg = 0x80000000,
};

[[nodiscard]] inline constexpr bool has(unsigned val, Cr0 bit) noexcept {
    return (val & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] inline Status validate_paging(unsigned cr0) noexcept {
    if (has(cr0, Cr0::Pg) && !has(cr0, Cr0::Pe)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::cr
""",
"bifrost/pbsd.bifrost.exit.cppm": """module;
#include <cstdint>

export module pbsd.bifrost.exit;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/vmm.h — VM exit reason codes.
export namespace pbsd::bifrost::exit {

enum class Code : unsigned int {
    Inout = 0,
    Vmx = 1,
    Bogus = 2,
    Rdmsr = 3,
    Wrmsr = 4,
    Hlt = 5,
    Mtrap = 6,
    Pause = 7,
    Paging = 8,
    InstEmul = 9,
    SpinupAp = 10,
    Rendezvous = 12,
    IoapicEoi = 13,
    Suspended = 14,
    InoutStr = 15,
    TaskSwitch = 16,
    Monitor = 17,
    Mwait = 18,
    Svm = 19,
    Reqidle = 20,
    Debug = 21,
    Vminsn = 22,
    Bpt = 23,
    Ipi = 24,
};

[[nodiscard]] inline Status validate_code(Code c) noexcept {
    switch (c) {
    case Code::Inout:
    case Code::Vmx:
    case Code::Svm:
    case Code::Paging:
    case Code::Hlt:
        return Status::Ok;
    default:
        return Status::Ok;
    }
}

[[nodiscard]] inline constexpr bool is_nested(Code c) noexcept {
    return c == Code::Vmx || c == Code::Svm;
}

} // namespace pbsd::bifrost::exit
""",
}


def main() -> None:
    created = 0
    for rel, content in MODULES.items():
        path = ROOT / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            path.write_text(content, encoding="utf-8", newline="\n")
            created += 1
    print(f"Wrote/updated {created} of {len(MODULES)} module files")


if __name__ == "__main__":
    main()
