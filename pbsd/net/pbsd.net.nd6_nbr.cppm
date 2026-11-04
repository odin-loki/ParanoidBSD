module;
#include <cstdint>

export module pbsd.net.nd6_nbr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/nd6_nbr.c — ND6 neighbor cache states.
export namespace pbsd::net::nd6_nbr {

enum class State : unsigned char {
    Incomplete = 0,
    Reachable = 1,
    Stale = 2,
    Delay = 3,
    Probe = 4,
};

struct Entry {
    State state{State::Incomplete};
    unsigned probes{};
};

[[nodiscard]] inline Status transition(Entry& e, State next) noexcept {
    switch (e.state) {
    case State::Incomplete:
        if (next != State::Reachable && next != State::Probe) {
            return Status::Invalid;
        }
        break;
    case State::Reachable:
        if (next != State::Stale && next != State::Probe) {
            return Status::Invalid;
        }
        break;
    case State::Stale:
        if (next != State::Delay && next != State::Probe) {
            return Status::Invalid;
        }
        break;
    case State::Delay:
        if (next != State::Probe && next != State::Reachable) {
            return Status::Invalid;
        }
        break;
    case State::Probe:
        if (next != State::Reachable && next != State::Incomplete) {
            return Status::Invalid;
        }
        break;
    }
    if (next == State::Probe) {
        ++e.probes;
    }
    e.state = next;
    return Status::Ok;
}

} // namespace pbsd::net::nd6_nbr
