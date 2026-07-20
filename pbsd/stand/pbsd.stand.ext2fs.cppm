module;
#include <cstddef>

export module pbsd.stand.ext2fs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/ext2fs.c
export namespace pbsd::stand::ext2fs {

inline constexpr unsigned kBlockSize = 4096;
inline constexpr unsigned kInodeSize = 256;

[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    if (size != 1024 && size != 2048 && size != 4096) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::ext2fs
