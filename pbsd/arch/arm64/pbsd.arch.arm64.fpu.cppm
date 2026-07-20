module;
#include <cstdint>

export module pbsd.arch.arm64.fpu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/armreg.h — FP/SIMD enable bits.
export namespace pbsd::arch::arm64::fpu {

inline constexpr unsigned long long kFpenEnable = 0x3ull << 20;
inline constexpr unsigned long long kFpenDisable = 0x0ull;

enum class FpExc : unsigned char {
    Inexact = 0,
    Underflow = 1,
    Overflow = 2,
    Invalid = 3,
    DivideByZero = 4,
};

[[nodiscard]] inline Status validate_fpen(unsigned long long cpacr) noexcept {
    const auto fpen = (cpacr >> 20) & 0x3ull;
    return fpen <= 0x3ull ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool fp_enabled(unsigned long long cpacr) noexcept {
    return ((cpacr >> 20) & 0x3ull) == 0x3ull;
}

} // namespace pbsd::arch::arm64::fpu
