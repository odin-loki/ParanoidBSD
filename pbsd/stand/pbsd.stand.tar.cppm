module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.tar;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/tar.c
export namespace pbsd::stand::tar {

inline constexpr unsigned kBlockSize = 512;
[[nodiscard]] inline Status validate_block_size(unsigned size) noexcept {
    return size == kBlockSize ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::tar
