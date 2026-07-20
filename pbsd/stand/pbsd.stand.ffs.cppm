module;
#include <cstddef>

export module pbsd.stand.ffs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/ffs.c
export namespace pbsd::stand::ffs {

inline constexpr unsigned kBlockSize = 4096;
inline constexpr unsigned kDirectBlocks = 12;

[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    if (size != 1024 && size != 2048 && size != 4096 && size != 8192) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::ffs
