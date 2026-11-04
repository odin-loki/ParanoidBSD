module;
#include <cstdint>

export module pbsd.arch.arm64.el2;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/hyp.c
export namespace pbsd::arch::arm64::el2 {

inline constexpr unsigned long long kHcrEl2Default = 0x8000'0000ull; // RW

enum class Trap : unsigned char {
    None = 0,
    Wfi = 1,
    Wfe = 2,
    Msr = 3,
};

[[nodiscard]] inline Status validate_trap(unsigned t) noexcept {
    return t <= static_cast<unsigned>(Trap::Msr) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_hcr(unsigned long long hcr) noexcept {
    if ((hcr & 0xFFFF'FFFF'0000'0000ull) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::el2
