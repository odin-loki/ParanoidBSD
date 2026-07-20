module;
#include <cstdint>

export module pbsd.net.lacp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/ieee8023ad_lacp.h — LACP state bits and timers.
export namespace pbsd::net::lacp {

inline constexpr unsigned kNTimer = 3;

enum class Timer : unsigned char {
    CurrentWhile = 0,
    Periodic     = 1,
    WaitWhile    = 2,
};

enum class State : unsigned char {
    Activity     = 1u << 0,
    Timeout      = 1u << 1,
    Aggregation  = 1u << 2,
    Sync         = 1u << 3,
    Collecting   = 1u << 4,
    Distributing = 1u << 5,
    Defaulted    = 1u << 6,
    Expired      = 1u << 7,
};

[[nodiscard]] inline Status validate_timer(Timer t) noexcept {
    if (static_cast<unsigned>(t) >= kNTimer) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool state_has(unsigned s, State bit) noexcept {
    return (s & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::lacp
