module;
#include <cstdint>

export module pbsd.net.tcp_hpts;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_hpts.h — HPTS slot timing helpers.
export namespace pbsd::net::tcp_hpts {

inline constexpr unsigned kUsecsPerSlot = 10;
inline constexpr unsigned kUsecInSec = 1000000;
inline constexpr unsigned kMsecInSec = 1000;
inline constexpr unsigned kUsecInMsec = 1000;

struct TimeVal {
    long sec{};
    long usec{};
};

[[nodiscard]] inline unsigned ms_to_slots(unsigned ms) noexcept {
    return ms * 100u + 1u;
}

[[nodiscard]] inline unsigned usec_to_slots(unsigned usec) noexcept {
    return (usec + 9u) / kUsecsPerSlot;
}

[[nodiscard]] inline unsigned tv_to_hpts_slot(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * 100000u
         + static_cast<unsigned>(sv.usec / kUsecsPerSlot);
}

[[nodiscard]] inline unsigned tv_to_usec(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * kUsecInSec
         + static_cast<unsigned>(sv.usec);
}

[[nodiscard]] inline unsigned tv_to_msec(TimeVal const& sv) noexcept {
    return static_cast<unsigned>(sv.sec) * kMsecInSec
         + static_cast<unsigned>(sv.usec / kUsecInMsec);
}

} // namespace pbsd::net::tcp_hpts
