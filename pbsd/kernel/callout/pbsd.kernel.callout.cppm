module;
#include <cstdint>

export module pbsd.kernel.callout;

export import pbsd.core;

/// Wave 4/5 — callout wheel from sys/callout.h, kern/kern_timeout.c.
export namespace pbsd::kernel::callout {

inline constexpr unsigned kCalloutTickhz = 1000;
inline constexpr unsigned kCalloutWheelSize = 512;

enum class CalloutState : unsigned char {
    Idle = 0,
    Active = 1,
    Pending = 2,
    Running = 3,
};

struct Callout {
    CalloutState state{CalloutState::Idle};
    std::uint64_t ticks{};
    std::uint64_t deadline{};
    bool oneshot{};
};

[[nodiscard]] constexpr Status validate_ticks(std::uint64_t ticks) noexcept {
    if (ticks == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr std::uint64_t deadline_from_now(std::uint64_t now,
                                                          std::uint64_t ticks) noexcept {
    return now + ticks;
}

[[nodiscard]] constexpr bool is_expired(std::uint64_t now, std::uint64_t deadline) noexcept {
    return now >= deadline;
}

[[nodiscard]] constexpr bool can_schedule(CalloutState s) noexcept {
    return s == CalloutState::Idle || s == CalloutState::Pending;
}

} // namespace pbsd::kernel::callout
