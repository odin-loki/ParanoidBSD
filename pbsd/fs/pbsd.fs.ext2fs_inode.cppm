module;
#include <cstdint>

export module pbsd.fs.ext2fs_inode;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/ext2fs/ext2_inode.c — ext2 inode layout helpers.
export namespace pbsd::fs::ext2fs_inode {

inline constexpr unsigned kInodeSize = 128;
inline constexpr unsigned kExtInodeSize = 256;

enum class FileType : unsigned char {
    Unknown = 0,
    Regular = 1,
    Directory = 2,
    Symlink = 7,
};

struct InodeLite {
    unsigned mode{};
    unsigned uid{};
    unsigned gid{};
    std::uint64_t size{};
    FileType type{FileType::Unknown};
};

[[nodiscard]] inline FileType mode_to_type(unsigned mode) noexcept {
    switch (mode & 0xF000) {
    case 0x8000: return FileType::Regular;
    case 0x4000: return FileType::Directory;
    case 0xA000: return FileType::Symlink;
    default: return FileType::Unknown;
    }
}

[[nodiscard]] inline Status validate_inode(const InodeLite& ino) noexcept {
    if (ino.size == 0 && ino.type == FileType::Regular) {
        return Status::Ok;
    }
    if (ino.type == FileType::Unknown) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::ext2fs_inode
