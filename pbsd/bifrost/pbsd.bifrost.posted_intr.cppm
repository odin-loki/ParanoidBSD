module;
#include <cstdint>

export module pbsd.bifrost.posted_intr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/posted_intr.c
export namespace pbsd::bifrost::posted_intr {

inline constexpr unsigned kVectorMax = 255;
[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec <= kVectorMax ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::posted_intr
