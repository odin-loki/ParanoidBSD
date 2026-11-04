module;
#include <cstdint>

export module pbsd.net.ndp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/nd6.h — ND6 neighbor cache states.
export namespace pbsd::net::ndp {

enum class LlState : signed char {
    NoState    = -2,
    Incomplete = 0,
    Reachable  = 1,
    Stale      = 2,
    Delay      = 3,
    Probe      = 4,
};

enum class Flag : unsigned int {
    PerformNud = 0x00000001,
    AcceptRtadv = 0x00000002,
    PreferIflladdr = 0x00000004,
};

[[nodiscard]] inline Status validate_state(LlState s) noexcept {
    switch (s) {
    case LlState::Incomplete:
    case LlState::Reachable:
    case LlState::Stale:
    case LlState::Delay:
    case LlState::Probe:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool is_probreach(LlState s) noexcept {
    return static_cast<signed char>(s) > static_cast<signed char>(LlState::Incomplete);
}

} // namespace pbsd::net::ndp
