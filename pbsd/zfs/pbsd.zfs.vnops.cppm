module;
#include <cstdint>

export module pbsd.zfs.vnops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/zfs_vnops.c — ZFS vnode op flags.
export namespace pbsd::zfs::vnops {

enum class OpFlag : unsigned {
    ReadSync  = 0x0001,
    WriteSync = 0x0002,
    Snapshot  = 0x0004,
    Clone     = 0x0008,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(OpFlag::ReadSync)
                            | static_cast<unsigned>(OpFlag::WriteSync)
                            | static_cast<unsigned>(OpFlag::Snapshot)
                            | static_cast<unsigned>(OpFlag::Clone);
    if ((flags & ~kAll) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool needs_sync(unsigned flags, bool writing) noexcept {
    if (writing) {
        return (flags & static_cast<unsigned>(OpFlag::WriteSync)) != 0;
    }
    return (flags & static_cast<unsigned>(OpFlag::ReadSync)) != 0;
}

} // namespace pbsd::zfs::vnops
