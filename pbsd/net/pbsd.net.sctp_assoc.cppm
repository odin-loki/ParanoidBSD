module;
#include <cstdint>

export module pbsd.net.sctp_assoc;

import pbsd.core;
import pbsd.net.sctp;

/// PROVENANCE: hbsd/src/sys/netinet/sctp_constants.h — SCTP association states.
export namespace pbsd::net::sctp::assoc {

enum class State : unsigned short {
    Empty            = 0x0000,
    Inuse            = 0x0001,
    CookieWait       = 0x0002,
    CookieEchoed     = 0x0004,
    Open             = 0x0008,
    ShutdownSent     = 0x0010,
    ShutdownReceived = 0x0020,
    ShutdownAckSent  = 0x0040,
    ShutdownPending  = 0x0080,
    ClosedSocket     = 0x0100,
};

inline constexpr unsigned short kStateMask = 0x007f;

[[nodiscard]] inline unsigned short core_state(unsigned short raw) noexcept {
    return raw & kStateMask;
}

[[nodiscard]] inline bool is_established(unsigned short raw) noexcept {
    return core_state(raw) == static_cast<unsigned short>(State::Open);
}

[[nodiscard]] inline bool is_cookie_phase(unsigned short raw) noexcept {
    auto s = core_state(raw);
    return s == static_cast<unsigned short>(State::CookieWait)
        || s == static_cast<unsigned short>(State::CookieEchoed);
}

[[nodiscard]] inline Status validate_transition(unsigned short from,
                                                unsigned short to) noexcept {
    if (core_state(from) == static_cast<unsigned short>(State::Empty)
        && core_state(to) == static_cast<unsigned short>(State::Inuse)) {
        return Status::Ok;
    }
    if (is_cookie_phase(from) && core_state(to) == static_cast<unsigned short>(State::Open)) {
        return Status::Ok;
    }
    if (core_state(from) == static_cast<unsigned short>(State::Open)
        && core_state(to) >= static_cast<unsigned short>(State::ShutdownSent)) {
        return Status::Ok;
    }
    return Status::Protocol;
}

struct Info {
    unsigned short state{};
    unsigned short inbound_streams{};
    unsigned short outbound_streams{};
};

[[nodiscard]] inline Status validate_info(Info const& info) noexcept {
    if (info.inbound_streams == 0 || info.outbound_streams == 0) {
        return Status::Invalid;
    }
    if (core_state(info.state) > static_cast<unsigned short>(State::ShutdownAckSent)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::sctp::assoc
