module;
#include <cstdint>

export module pbsd.zfs.ioctl;

import pbsd.core;
import pbsd.rights;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/fs/zfs.h, zfs_ioctl.h
export namespace pbsd::zfs::ioctl {

enum class ZfsType : unsigned int {
    Invalid    = 0,
    Filesystem = 1u << 0,
    Snapshot   = 1u << 1,
    Volume     = 1u << 2,
    Pool       = 1u << 3,
    Bookmark   = 1u << 4,
    Vdev       = 1u << 5,
    Dataset    = Filesystem | Volume | Snapshot,
};

enum class Ioc : unsigned int {
    First              = ('Z' << 8),
    PoolCreate         = First,       // 0x5a00
    PoolDestroy        = First + 1,
    PoolImport         = First + 2,
    PoolExport         = First + 3,
    PoolConfigs        = First + 4,
    PoolStats          = First + 5,
    PoolTryimport      = First + 6,
    ObjsetStats        = First + 0x12,
    SetProp            = First + 0x16,
    Create             = First + 0x17,
    Destroy            = First + 0x18,
    Snapshot           = First + 0x23,
    LoadKey            = First + 0x49,
    UnloadKey          = First + 0x4a,
    Jail               = First + 0x85, // FreeBSD ZFS_IOC_JAIL
    SetBootenv         = First + 0x87,
    GetBootenv         = First + 0x88,
    Last,
};

/// DMU backup stream feature flags (zfs_ioctl.h).
enum class BackupFeature : unsigned long long {
    Dedup               = 1ull << 0,
    DedupProps          = 1ull << 1,
    SaSpill             = 1ull << 2,
    EmbedData           = 1ull << 16,
    Lz4                 = 1ull << 17,
    LargeBlocks         = 1ull << 19,
    Resuming            = 1ull << 20,
    Redacted            = 1ull << 21,
    Compressed          = 1ull << 22,
    LargeDnode          = 1ull << 23,
    Raw                 = 1ull << 24,
    Zstd                = 1ull << 25,
    Holds               = 1ull << 26,
    SwitchToLargeBlocks = 1ull << 27,
    Longname            = 1ull << 28,
};

enum class Snapdir : int {
    Hidden   = 0, // ZFS_SNAPDIR_HIDDEN
    Visible  = 1,
    Disabled = 2,
};

enum class AclType : int {
    Off    = 0,
    Posix  = 1,
    Nfsv4  = 2,
};

[[nodiscard]] inline constexpr CapabilityRights rights_for_ioc(Ioc cmd) noexcept {
    switch (cmd) {
    case Ioc::PoolCreate:
    case Ioc::PoolDestroy:
    case Ioc::PoolImport:
    case Ioc::Create:
    case Ioc::Destroy:
    case Ioc::Snapshot:
    case Ioc::LoadKey:
    case Ioc::UnloadKey:
        return CapabilityRights::Read | CapabilityRights::Write | CapabilityRights::Grant;
    case Ioc::PoolStats:
    case Ioc::ObjsetStats:
    case Ioc::GetBootenv:
        return CapabilityRights::Read;
    case Ioc::SetProp:
    case Ioc::SetBootenv:
        return CapabilityRights::Read | CapabilityRights::Write;
    case Ioc::Jail:
        return CapabilityRights::Read | CapabilityRights::Map | CapabilityRights::Grant;
    default:
        return CapabilityRights::Read;
    }
}

[[nodiscard]] inline Status check_ioc(CapabilityRights have, Ioc cmd) noexcept {
    CapabilityRights need = rights_for_ioc(cmd);
    if (!rights_subset(need, have)) {
        return Status::Denied;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool feature_enabled(unsigned long long flags,
                                                    BackupFeature f) noexcept {
    return (flags & static_cast<unsigned long long>(f)) != 0;
}

} // namespace pbsd::zfs::ioctl
