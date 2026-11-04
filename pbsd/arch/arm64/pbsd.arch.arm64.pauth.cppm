module;
#include <cstdint>

export module pbsd.arch.arm64.pauth;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/pauth.c
export namespace pbsd::arch::arm64::pauth {

inline constexpr unsigned kKeySlots = 5;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kKeySlots ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::pauth
