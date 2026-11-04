module;
#include <cstdint>

export module pbsd.kernel.timeout;

export import pbsd.core;

/// Wave 4/5 — timeout API from sys/timeout.h, kern/kern_timeout.c.
export namespace pbsd::kernel::timeout {

inline constexpr unsigned kTimeoutTickhz = 1000;
inline constexpr unsigned kTimeoutMaxSec = 86400;

enum class TimeoutState : unsigned char {
    Unscheduled = 0,
    Scheduled = 1,
    Fired = 2,
    Cancelled = 3,
};

struct Timeout {
    TimeoutState state{TimeoutState::Unscheduled};
    std::uint64_t  deadline_ticks{};
    bool           absolute{};
};

[[nodiscard]] constexpr Status validate_interval_sec(unsigned sec) noexcept {
    if (sec == 0 || sec > kTimeoutMaxSec) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr std::uint64_t sec_to_ticks(unsigned sec) noexcept {
    return static_cast<std::uint64_t>(sec) * kTimeoutTickhz;
}

[[nodiscard]] constexpr bool is_pending(TimeoutState s) noexcept {
    return s == TimeoutState::Scheduled;
}

[[nodiscard]] constexpr bool is_fired(TimeoutState s) noexcept {
    return s == TimeoutState::Fired;
}

} // namespace pbsd::kernel::timeout
