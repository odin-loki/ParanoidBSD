module;
#include <cstdint>

export module pbsd.arch.amd64.fpu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/fpu.h
export namespace pbsd::arch::amd64::fpu {

inline constexpr std::uint16_t kInitialFpucw = 0x037F;
inline constexpr std::uint32_t kInitialMxcsr = 0x1F80;
inline constexpr std::uint32_t kInitialMxcsrMask = 0xFFBF;
inline constexpr std::uint64_t kXstateCompact = 1ull << 63;

enum class RoundingMode : unsigned char {
    Nearest = 0,
    Down = 1,
    Up = 2,
    TowardZero = 3,
};

[[nodiscard]] inline Status validate_mxcsr(std::uint32_t mxcsr) noexcept {
    if ((mxcsr & ~kInitialMxcsrMask) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rounding(RoundingMode mode) noexcept {
    return static_cast<unsigned>(mode) <= 3 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::fpu
