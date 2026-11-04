module;
#include <cstdint>

export module pbsd.arch.arm64.pac;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/pac.c
export namespace pbsd::arch::arm64::pac {

inline constexpr unsigned kKeyLen = 16;
[[nodiscard]] inline Status validate_key_len(unsigned len) noexcept {
    return len == kKeyLen ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::pac
