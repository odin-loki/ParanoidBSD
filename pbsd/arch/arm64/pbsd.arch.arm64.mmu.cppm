module;
#include <cstdint>

export module pbsd.arch.arm64.mmu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/pmap.c
export namespace pbsd::arch::arm64::mmu {

inline constexpr unsigned kPageShift = 12;
inline constexpr unsigned kTtbrLevels = 4;

[[nodiscard]] inline Status validate_level(unsigned level) noexcept {
    return level < kTtbrLevels ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::mmu
