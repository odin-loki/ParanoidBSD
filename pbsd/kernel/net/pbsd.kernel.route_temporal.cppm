module;
#include <cstdint>

export module pbsd.kernel.route_temporal;

import pbsd.core;

/// Freestanding port of `net/route/route_temporal.c` temporal route expiry.
export namespace pbsd::kernel::route_temporal {

[[nodiscard]] inline bool is_expired(std::uint32_t nh_expire,
                                     std::int64_t time_uptime) noexcept {
    if (nh_expire == 0) {
        return false;
    }
    return nh_expire <= static_cast<std::uint32_t>(time_uptime);
}

[[nodiscard]] inline std::int64_t next_callout(std::int64_t current_next,
                                               std::uint32_t nh_expire,
                                               std::int64_t time_uptime) noexcept {
    if (nh_expire == 0 || nh_expire <= static_cast<std::uint32_t>(time_uptime)) {
        return current_next;
    }
    const auto expire = static_cast<std::int64_t>(nh_expire);
    if (current_next == 0 || current_next > expire) {
        return expire;
    }
    return current_next;
}

struct ExpireScan {
    std::int64_t next_callout{};
};

[[nodiscard]] inline bool scan_route(ExpireScan& scan, std::uint32_t nh_expire,
                                     std::int64_t time_uptime) noexcept {
    if (is_expired(nh_expire, time_uptime)) {
        return true;
    }
    scan.next_callout = next_callout(scan.next_callout, nh_expire, time_uptime);
    return false;
}

} // namespace pbsd::kernel::route_temporal
