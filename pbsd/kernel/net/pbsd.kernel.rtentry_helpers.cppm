module;
#include <cstdint>

export module pbsd.kernel.rtentry_helpers;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/route.c — rtentry flag helpers.
export namespace pbsd::kernel::rtentry_helpers {

enum class RtFlag : unsigned {
    None = 0,
    Up = 0x0001,
    Gateway = 0x0002,
    Host = 0x0004,
    Reject = 0x0008,
    Dynamic = 0x0010,
    Modified = 0x0020,
    Done = 0x0040,
    Mask = 0x0080,
    Cloning = 0x0100,
    Xresolve = 0x0200,
    LLinfo = 0x0400,
    Static = 0x0800,
    Blackhole = 0x1000,
    Proto1 = 0x4000,
    Proto2 = 0x8000,
};

[[nodiscard]] constexpr RtFlag operator|(RtFlag a, RtFlag b) noexcept {
    return static_cast<RtFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has(RtFlag set, RtFlag bit) noexcept {
    return (static_cast<unsigned>(set) & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] inline Status validate_flags(RtFlag f) noexcept {
    if (has(f, RtFlag::Reject) && has(f, RtFlag::Gateway)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_host(RtFlag f) noexcept {
    return has(f, RtFlag::Host);
}

} // namespace pbsd::kernel::rtentry_helpers
