module;
#include <cstdint>

export module pbsd.bifrost.vhpet;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vhpet.h — virtual HPET MMIO window.
export namespace pbsd::bifrost::vhpet {

inline constexpr std::uint64_t kBase = 0xFED0'0000ull;
inline constexpr unsigned kSize = 1024;
inline constexpr unsigned kNumTimers = 3;

[[nodiscard]] inline Status validate_gpa(std::uint64_t gpa) noexcept {
    if (gpa < kBase || gpa >= kBase + kSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_timer(unsigned timer) noexcept {
    return timer < kNumTimers ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::vhpet
