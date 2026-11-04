module;
#include <cstdint>

export module pbsd.zfs.vfsops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/zfs_vfsops.c — ZFS mount ops.
export namespace pbsd::zfs::vfsops {

enum class MountFlag : unsigned {
    ReadOnly = 0x0001,
    NoSetuid = 0x0002,
    NoExec   = 0x0004,
    Snapshot = 0x0008,
};

struct MountCtx {
    unsigned flags{};
    bool mounted{false};
};

[[nodiscard]] inline Status validate_mount(const MountCtx& ctx) noexcept {
    if (ctx.mounted) {
        return Status::Busy;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_readonly(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(MountFlag::ReadOnly)) != 0;
}

[[nodiscard]] inline bool is_snapshot_mount(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(MountFlag::Snapshot)) != 0;
}

} // namespace pbsd::zfs::vfsops
