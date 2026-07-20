module;
#include <cstdint>

export module pbsd.arch.arm64.sve;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/sve.c
export namespace pbsd::arch::arm64::sve {

inline constexpr unsigned kMinVectorLen = 128;
inline constexpr unsigned kMaxVectorLen = 2048;

[[nodiscard]] inline Status validate_vector_len(unsigned len) noexcept {
    if (len < kMinVectorLen || len > kMaxVectorLen || (len % 128) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::sve
