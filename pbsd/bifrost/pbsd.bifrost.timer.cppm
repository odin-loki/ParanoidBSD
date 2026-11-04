module;
#include <cstdint>

export module pbsd.bifrost.timer;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vhpet.c
export namespace pbsd::bifrost::timer {

inline constexpr unsigned kDefaultHz = 100000000;
inline constexpr unsigned kMinPeriodNs = 1000;

[[nodiscard]] inline Status validate_period_ns(unsigned period) noexcept {
    return period >= kMinPeriodNs ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::timer
