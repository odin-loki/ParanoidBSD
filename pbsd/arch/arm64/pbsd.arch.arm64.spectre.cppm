module;
#include <cstdint>

export module pbsd.arch.arm64.spectre;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/spectre.c
export namespace pbsd::arch::arm64::spectre {

enum class Mitigation : unsigned char {
    None = 0,
    Bti = 1,
    Pauth = 2,
    Ssb = 3,
};

[[nodiscard]] inline Status validate_mitigation(unsigned m) noexcept {
    return m <= static_cast<unsigned>(Mitigation::Ssb) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool requires_ssbd(Mitigation m) noexcept {
    return m == Mitigation::Ssb;
}

} // namespace pbsd::arch::arm64::spectre
