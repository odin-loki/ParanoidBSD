module;
#include <cstdint>

export module pbsd.arch.arm64.bti;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/bti.c
export namespace pbsd::arch::arm64::bti {

enum class Guard : unsigned char {
    None = 0,
    Bti = 1,
};

[[nodiscard]] inline Status validate_guard(unsigned guard) noexcept {
    return guard <= static_cast<unsigned>(Guard::Bti) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::arm64::bti
