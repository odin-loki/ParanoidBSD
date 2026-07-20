module;
#include <cstdint>

export module pbsd.net.tcp_timer_wheel;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_timer.c — timer wheel slot scaffold.
export namespace pbsd::net::tcp_timer_wheel {

inline constexpr unsigned kSlots = 32;

struct Wheel {
    std::uint64_t ticks{};
    unsigned armed{};
};

[[nodiscard]] inline Status arm(Wheel& w, unsigned slot) noexcept {
    if (slot >= kSlots) {
        return Status::Invalid;
    }
    w.armed |= (1u << slot);
    return Status::Ok;
}

[[nodiscard]] inline Status tick(Wheel& w) noexcept {
    ++w.ticks;
    return Status::Ok;
}

[[nodiscard]] inline bool is_armed(const Wheel& w, unsigned slot) noexcept {
    if (slot >= kSlots) {
        return false;
    }
    return (w.armed & (1u << slot)) != 0;
}

} // namespace pbsd::net::tcp_timer_wheel
