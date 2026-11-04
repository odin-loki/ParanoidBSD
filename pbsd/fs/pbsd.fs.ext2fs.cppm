module;
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
