module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.sha256;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/sha256.c
export namespace pbsd::stand::sha256 {

inline constexpr unsigned kDigestBytes = 32;
[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestBytes ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::sha256
