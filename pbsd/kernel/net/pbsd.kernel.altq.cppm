module;
#include <cstdint>

export module pbsd.kernel.altq;

import pbsd.core;

/// Freestanding port of `net/altq/altq_subr.c` — ALTQ discipline helpers.
export namespace pbsd::kernel::altq {

inline constexpr unsigned kAltqPfifo = 0u;
inline constexpr unsigned kAltqCbq   = 1u;
inline constexpr unsigned kAltqHfsc  = 2u;
inline constexpr unsigned kAltqFaq   = 3u;
inline constexpr unsigned kAltqCodel = 4u;

struct Discipline {
    unsigned type{kAltqPfifo};
    unsigned bandwidth_bps{};
    unsigned queue_limit{50};
    bool     enabled{};
};

[[nodiscard]] inline Status validate_type(unsigned type) noexcept {
    if (type > kAltqCodel) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(Discipline& d, unsigned type) noexcept {
    if (validate_type(type) != Status::Ok) {
        return Status::Invalid;
    }
    d.type = type;
    d.bandwidth_bps = 0;
    d.queue_limit = 50;
    d.enabled = false;
    return Status::Ok;
}

[[nodiscard]] inline Status enable(Discipline& d) noexcept {
    if (d.queue_limit == 0) {
        return Status::Invalid;
    }
    d.enabled = true;
    return Status::Ok;
}

} // namespace pbsd::kernel::altq
