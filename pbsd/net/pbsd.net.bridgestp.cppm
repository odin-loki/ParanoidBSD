module;
#include <cstdint>

export module pbsd.net.bridgestp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/bridgestp.h — STP port state and role constants.
export namespace pbsd::net::bridgestp {

enum class IfState : unsigned char {
    Disabled   = 0,
    Listening  = 1,
    Learning   = 2,
    Forwarding = 3,
    Blocking   = 4,
    Discarding = 5,
};

enum class Role : unsigned char {
    Disabled   = 0,
    Root       = 1,
    Designated = 2,
    Alternate  = 3,
    Backup     = 4,
};

enum class PortFlag : unsigned int {
    CanMigrate = 0x0001,
    NewInfo    = 0x0002,
    Disputed   = 0x0004,
    AdmCost    = 0x0008,
    AutoEdge   = 0x0010,
};

struct Port {
    IfState state{IfState::Disabled};
    Role    role{Role::Disabled};
    unsigned int flags{};
    unsigned int transitions{};
};

[[nodiscard]] inline Status validate_state(IfState st) noexcept {
    if (st > IfState::Discarding) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status transition(Port& port, IfState next) noexcept {
    if (validate_state(next) != Status::Ok) {
        return Status::Invalid;
    }
    if (port.state == next) {
        return Status::Ok;
    }
    port.state = next;
    ++port.transitions;
    return Status::Ok;
}

[[nodiscard]] inline bool forwards(IfState st) noexcept {
    return st == IfState::Forwarding;
}

[[nodiscard]] inline bool learns(IfState st) noexcept {
    return st == IfState::Learning || st == IfState::Forwarding;
}

} // namespace pbsd::net::bridgestp
