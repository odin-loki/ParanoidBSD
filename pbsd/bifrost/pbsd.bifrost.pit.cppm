module;
#include <cstdint>

export module pbsd.bifrost.pit;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/pit.c
export namespace pbsd::bifrost::pit {

inline constexpr unsigned kFrequencyHz = 1193182;
inline constexpr unsigned kChannel0 = 0;

[[nodiscard]] inline Status validate_divisor(unsigned divisor) noexcept {
    return divisor >= 1 && divisor <= 65535 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::pit
