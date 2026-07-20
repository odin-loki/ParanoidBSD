module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.verify;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/verify.c
export namespace pbsd::stand::verify {

inline constexpr unsigned kDigestLen = 32;

[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestLen ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::verify
