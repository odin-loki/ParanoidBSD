module;
#include <cstdint>

export module pbsd.net.tcp_timer;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_timer.h — TCP retransmit/persist/keep timers.
export namespace pbsd::net::tcp_timer {

enum class Kind : unsigned char {
    Rexmt  = 0,
    Persist = 1,
    Keep   = 2,
    _2msl  = 3,
    Delack = 4,
};

inline constexpr unsigned kTimerCount = 5;
inline constexpr unsigned kPersMin    = 5000;  // TCPT_PERSMIN (ms)
inline constexpr unsigned kPersMax    = 60000; // TCPT_PERSMAX (ms)
inline constexpr unsigned kKeepInit   = 7200000;
inline constexpr unsigned kKeepIdle   = 7200000;
inline constexpr unsigned kKeepintvl  = 75000;
inline constexpr unsigned kMaxidle    = 86400000;

struct TimerSet {
    unsigned ticks[kTimerCount]{};
    bool     running[kTimerCount]{};
};

[[nodiscard]] inline Status validate_kind(Kind kind) noexcept {
    if (static_cast<unsigned>(kind) >= kTimerCount) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status start(TimerSet& ts, Kind kind, unsigned ticks) noexcept {
    if (validate_kind(kind) != Status::Ok || ticks == 0) {
        return Status::Invalid;
    }
    const auto idx = static_cast<unsigned>(kind);
    ts.ticks[idx] = ticks;
    ts.running[idx] = true;
    return Status::Ok;
}

[[nodiscard]] inline Status stop(TimerSet& ts, Kind kind) noexcept {
    if (validate_kind(kind) != Status::Ok) {
        return Status::Invalid;
    }
    const auto idx = static_cast<unsigned>(kind);
    ts.running[idx] = false;
    ts.ticks[idx] = 0;
    return Status::Ok;
}

[[nodiscard]] inline bool is_running(const TimerSet& ts, Kind kind) noexcept {
    if (validate_kind(kind) != Status::Ok) {
        return false;
    }
    return ts.running[static_cast<unsigned>(kind)];
}

} // namespace pbsd::net::tcp_timer
