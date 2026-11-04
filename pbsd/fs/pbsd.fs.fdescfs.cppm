module;
#include <cstdint>

export module pbsd.fs.fdescfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/fdescfs/fdesc.h — fdescfs mount and node ids.
export namespace pbsd::fs::fdescfs {

enum class MountFlag : int {
    Unmountf = 0x01,
    Linrdlnkf = 0x02,
    Nodup = 0x04,
    Rdlnkf = 0x08,
};

enum class NodeId : int {
    Root = 1,
    Desc = 3,
};

inline constexpr int kMaxFd = 65535;

[[nodiscard]] inline Status validate_node(NodeId id) noexcept {
    switch (id) {
    case NodeId::Root:
    case NodeId::Desc:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_fd(int fd) noexcept {
    if (fd < 0 || fd > kMaxFd) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_mount_flags(int flags) noexcept {
    int allowed = static_cast<int>(MountFlag::Unmountf)
        | static_cast<int>(MountFlag::Linrdlnkf)
        | static_cast<int>(MountFlag::Nodup)
        | static_cast<int>(MountFlag::Rdlnkf);
    if ((flags & ~allowed) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::fdescfs
