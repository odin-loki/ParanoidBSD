module;
#include <cstdint>

export module pbsd.arch.arm64.dcz;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/dcache.c
export namespace pbsd::arch::arm64::dcz {

inline constexpr unsigned kDczBlockBytes = 64;

[[nodiscard]] inline Status validate_block(unsigned bytes) noexcept {
    return bytes == kDczBlockBytes ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::dcz
