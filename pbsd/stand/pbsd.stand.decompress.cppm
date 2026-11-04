module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.decompress;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/decompress.c
export namespace pbsd::stand::decompress {

inline constexpr unsigned kWindowBits = 15;

[[nodiscard]] inline Status validate_window_bits(unsigned bits) noexcept {
    return bits >= 8 && bits <= kWindowBits ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::decompress
