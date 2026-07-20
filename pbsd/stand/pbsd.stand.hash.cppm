module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.hash;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/sha256.c
export namespace pbsd::stand::hash {

inline constexpr unsigned kBlockSize = 64;
inline constexpr unsigned kDigestSize = 32;

[[nodiscard]] inline Status validate_block(unsigned block) noexcept {
    return block == kBlockSize ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::hash
