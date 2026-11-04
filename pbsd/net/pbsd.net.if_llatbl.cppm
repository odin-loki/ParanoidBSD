module;
#include <cstdint>

export module pbsd.net.if_llatbl;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_llatbl.h — link-layer address table states.
export namespace pbsd::net::if_llatbl {

enum class State : signed char {
    Incomplete = 0,
    Reachable  = 1,
    Stale      = 2,
    Delay      = 3,
    Probe      = 4,
};

enum class Flag : unsigned int {
    Static = 0x00000001,
    Proxy  = 0x00000002,
    Pinned = 0x00000004,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::Incomplete:
    case State::Reachable:
    case State::Stale:
    case State::Delay:
    case State::Probe:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline bool is_reachable(State s) noexcept {
    return s == State::Reachable;
}

} // namespace pbsd::net::if_llatbl
