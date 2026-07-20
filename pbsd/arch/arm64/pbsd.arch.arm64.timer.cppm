module;
#include <cstdint>

export module pbsd.arch.arm64.timer;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/cpu.h — generic timer (cntfrq/cntvct).
export namespace pbsd::arch::arm64::timer {

inline constexpr unsigned kDefaultFrqHz = 24000000;
inline constexpr unsigned kMinFrqHz = 1000000;
inline constexpr unsigned kMaxFrqHz = 1000000000;

enum class Reg : unsigned char {
    CntfrqEl0 = 0,
    CntvctEl0 = 1,
    CntpctEl0 = 2,
    CntvCtlEl0 = 3,
    CntkCtlEl1 = 4,
};

struct Sample {
    std::uint64_t counter{0};
    std::uint64_t frequency{kDefaultFrqHz};
};

[[nodiscard]] inline Status validate_frequency(std::uint64_t hz) noexcept {
    if (hz < kMinFrqHz || hz > kMaxFrqHz) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_sample(const Sample& s) noexcept {
    if (validate_frequency(s.frequency) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t ticks_to_ns(std::uint64_t ticks,
                                                 std::uint64_t frq) noexcept {
    if (frq == 0) {
        return 0;
    }
    return (ticks * 1000000000ull) / frq;
}

[[nodiscard]] inline std::uint64_t ns_to_ticks(std::uint64_t ns,
                                                 std::uint64_t frq) noexcept {
    if (frq == 0) {
        return 0;
    }
    return (ns * frq) / 1000000000ull;
}

} // namespace pbsd::arch::arm64::timer
