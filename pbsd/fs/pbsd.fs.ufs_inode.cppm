module;
#include <cstdint>

export module pbsd.fs.ufs_inode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/ufs/ufs/inode.h — UFS inode layout helpers.
export namespace pbsd::fs::ufs_inode {

inline constexpr unsigned kDirectBlocks = 12;
inline constexpr unsigned kNindir = 3;

struct Inode {
    unsigned short mode{};
    unsigned short nlink{};
    unsigned long long size{};
    unsigned direct[kDirectBlocks]{};
};

[[nodiscard]] inline Status validate_mode(unsigned short mode) noexcept {
    if ((mode & ~07777u) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_inode(const Inode& ino) noexcept {
    if (validate_mode(ino.mode) != Status::Ok) {
        return Status::Invalid;
    }
    if (ino.nlink == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_dir(unsigned short mode) noexcept {
    return (mode & 0170000u) == 0040000u;
}

} // namespace pbsd::fs::ufs_inode
