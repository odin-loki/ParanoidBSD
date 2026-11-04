module;
#include <cstddef>
#include <cstdint>

export module pbsd.stand.md5;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/md5.c
export namespace pbsd::stand::md5 {

inline constexpr unsigned kDigestBytes = 16;
[[nodiscard]] inline Status validate_digest_len(unsigned len) noexcept {
    return len == kDigestBytes ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::md5
