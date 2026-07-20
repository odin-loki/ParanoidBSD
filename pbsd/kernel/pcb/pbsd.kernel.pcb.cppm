module;
#include <cstdint>

export module pbsd.kernel.pcb;

export import pbsd.core;

/// Wave 4 — TCP protocol control block flags/states (netinet/tcp_var.h, tcp_fsm.h).
export namespace pbsd::kernel::pcb {

enum class TcpState : unsigned char {
    Closed      = 0,
    Listen      = 1,
    SynSent     = 2,
    SynReceived = 3,
    Established = 4,
    CloseWait   = 5,
    FinWait1    = 6,
    Closing     = 7,
    LastAck     = 8,
    FinWait2    = 9,
    TimeWait    = 10,
};

enum class TcpFlag : unsigned {
    None          = 0,
    AckNow        = 0x00000001,
    DelAck        = 0x00000002,
    NoDelay       = 0x00000004,
    NoOpt         = 0x00000008,
    SentFin       = 0x00000010,
    ReqScale      = 0x00000020,
    RcvdScale     = 0x00000040,
    ReqTstmp      = 0x00000080,
    RcvdTstmp     = 0x00000100,
    SackPermit    = 0x00000200,
    NeedSyn       = 0x00000400,
    NeedFin       = 0x00000800,
    NoPush        = 0x00001000,
    FastRecovery  = 0x00100000,
    Tso           = 0x01000000,
    ClosedFlag    = 0x04000000,
};

struct StateEntry {
    TcpState    state{};
    const char* name{};
};

inline constexpr StateEntry kStateTable[] = {
    {TcpState::Closed,      "CLOSED"},
    {TcpState::Listen,      "LISTEN"},
    {TcpState::SynSent,     "SYN_SENT"},
    {TcpState::SynReceived, "SYN_RECEIVED"},
    {TcpState::Established, "ESTABLISHED"},
    {TcpState::CloseWait,   "CLOSE_WAIT"},
    {TcpState::FinWait1,    "FIN_WAIT_1"},
    {TcpState::Closing,     "CLOSING"},
    {TcpState::LastAck,     "LAST_ACK"},
    {TcpState::FinWait2,    "FIN_WAIT_2"},
    {TcpState::TimeWait,    "TIME_WAIT"},
};

struct TcpcbStub {
    TcpState    t_state{TcpState::Closed};
    unsigned    t_flags{};
    unsigned    t_flags2{};
};

[[nodiscard]] inline unsigned state_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kStateTable) / sizeof(kStateTable[0]));
}

[[nodiscard]] constexpr TcpFlag operator|(TcpFlag a, TcpFlag b) noexcept {
    return static_cast<TcpFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool has_flag(unsigned flags, TcpFlag bit) noexcept {
    return (flags & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] constexpr bool state_established(TcpState s) noexcept {
    return s == TcpState::Established || s == TcpState::CloseWait
        || s == TcpState::FinWait1 || s == TcpState::FinWait2
        || s == TcpState::Closing || s == TcpState::LastAck;
}

[[nodiscard]] constexpr Status validate_state(TcpState s) noexcept {
    return static_cast<unsigned char>(s) <= static_cast<unsigned char>(TcpState::TimeWait)
        ? Status::Ok
        : Status::Invalid;
}

[[nodiscard]] constexpr Status validate_transition(TcpState from, TcpState to) noexcept {
    if (validate_state(from) != Status::Ok || validate_state(to) != Status::Ok) {
        return Status::Invalid;
    }
    if (from == TcpState::Closed && to == TcpState::Listen) {
        return Status::Ok;
    }
    if (from == TcpState::Listen && to == TcpState::SynReceived) {
        return Status::Ok;
    }
    if (from == TcpState::SynSent && to == TcpState::Established) {
        return Status::Ok;
    }
    if (from == TcpState::Established && to == TcpState::FinWait1) {
        return Status::Ok;
    }
    if (from == to) {
        return Status::Ok;
    }
    return Status::Protocol;
}

[[nodiscard]] inline Result<const char*> state_name(TcpState s) noexcept {
    for (const auto& e : kStateTable) {
        if (e.state == s) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

} // namespace pbsd::kernel::pcb
