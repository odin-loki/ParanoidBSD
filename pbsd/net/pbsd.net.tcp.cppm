module;
#include <cstdint>

export module pbsd.net.tcp;

import pbsd.core;
import pbsd.rights;
import pbsd.net.sockopt;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h — TCP socket option names and cap table.
export namespace pbsd::net::tcp {

enum class Flag : unsigned short {
    Fin  = 0x01,
    Syn  = 0x02,
    Rst  = 0x04,
    Push = 0x08,
    Ack  = 0x10,
    Urg  = 0x20,
    Ece  = 0x40,
    Cwr  = 0x80,
};

/// TCP FSM states (TCPS_* from hbsd/src/sys/netinet/tcp_fsm.h).
enum class State : unsigned char {
    Closed       = 0,
    Listen       = 1,
    SynSent      = 2,
    SynReceived  = 3,
    Established  = 4,
    CloseWait    = 5,
    FinWait1     = 6,
    Closing      = 7,
    LastAck      = 8,
    FinWait2     = 9,
    TimeWait     = 10,
};

struct StateEntry {
    State       state{};
    const char* name{};
};

inline constexpr StateEntry kStateTable[] = {
    {State::Closed, "CLOSED"},
    {State::Listen, "LISTEN"},
    {State::SynSent, "SYN_SENT"},
    {State::SynReceived, "SYN_RECEIVED"},
    {State::Established, "ESTABLISHED"},
    {State::CloseWait, "CLOSE_WAIT"},
    {State::FinWait1, "FIN_WAIT_1"},
    {State::Closing, "CLOSING"},
    {State::LastAck, "LAST_ACK"},
    {State::FinWait2, "FIN_WAIT_2"},
    {State::TimeWait, "TIME_WAIT"},
};

enum class Option : int {
    NoDelay          = 1,
    MaxSeg           = 2,
    NoPush           = 4,
    NoOpt            = 8,
    Md5Sig           = 16,
    Info             = 32,
    Stats            = 33,
    Log              = 34,
    LogBuf           = 35,
    LogId            = 36,
    TxtlsEnable      = 39,
    TxtlsMode        = 40,
    RxtlsEnable      = 41,
    RxtlsMode        = 42,
    Congestion       = 64,
    CcalgoOpt        = 65,
    MaxUnackTime     = 68,
    IdleReduce       = 70,
    RemoteUdpEncaps  = 71,
    DelAck           = 72,
    KeepInit         = 128,
    KeepIdle         = 256,
    KeepIntvl        = 512,
    KeepCnt          = 1024,
    FastOpen         = 1025,
    LogDump          = 37,
    LogDumpId        = 38,
    IwndNb           = 43,
    IwndNseg         = 44,
    UseDdp           = 45,
    LogIdCnt         = 46,
    LogTag           = 47,
    UserLog          = 48,
    FinIsRst         = 73,
    LogLimit         = 74,
    FunctionBlk      = 8192,
};

inline constexpr int kMssDefault  = 536;
inline constexpr int kMinMss      = 216;
inline constexpr int kMaxWin      = 65535;
inline constexpr int kMaxWinShift = 14;

struct OptionEntry {
    Option          name{};
    CapabilityRights need_read{CapabilityRights::None};
    CapabilityRights need_write{CapabilityRights::None};
    bool            kernel_only{false};
};

inline constexpr OptionEntry kTcpOptionTable[] = {
    {Option::NoDelay, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::MaxSeg, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::NoPush, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::Info, CapabilityRights::Read, CapabilityRights::None, false},
    {Option::Stats, CapabilityRights::Read, CapabilityRights::None, false},
    {Option::Congestion, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::KeepInit, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::KeepIdle, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::KeepIntvl, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::KeepCnt, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::FastOpen, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::Md5Sig, CapabilityRights::Read, CapabilityRights::Write, true},
    {Option::Log, CapabilityRights::Read, CapabilityRights::Write, true},
    {Option::LogBuf, CapabilityRights::Read, CapabilityRights::None, false},
    {Option::LogId, CapabilityRights::Read, CapabilityRights::Write, true},
    {Option::TxtlsEnable, CapabilityRights::Read, CapabilityRights::Write, true},
    {Option::RxtlsEnable, CapabilityRights::Read, CapabilityRights::Write, true},
    {Option::CcalgoOpt, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::MaxUnackTime, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::IdleReduce, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::RemoteUdpEncaps, CapabilityRights::Read, CapabilityRights::Write, false},
    {Option::DelAck, CapabilityRights::Read, CapabilityRights::Write, false},
};

[[nodiscard]] inline constexpr std::size_t option_table_size() noexcept {
    return sizeof(kTcpOptionTable) / sizeof(kTcpOptionTable[0]);
}

[[nodiscard]] inline CapabilityRights rights_for(Option opt, sockopt::OptDir dir) noexcept {
    for (auto const& e : kTcpOptionTable) {
        if (e.name != opt) {
            continue;
        }
        if (dir == sockopt::OptDir::Set) {
            return e.need_write != CapabilityRights::None ? e.need_write : e.need_read;
        }
        return e.need_read;
    }
    return dir == sockopt::OptDir::Set ? CapabilityRights::Write : CapabilityRights::Read;
}

[[nodiscard]] inline constexpr bool flags_has(unsigned short val, Flag f) noexcept {
    return (val & static_cast<unsigned short>(f)) != 0;
}

[[nodiscard]] inline constexpr bool is_syn_ack(unsigned short val) noexcept {
    return flags_has(val, Flag::Syn) && flags_has(val, Flag::Ack);
}

[[nodiscard]] inline constexpr std::size_t state_table_size() noexcept {
    return sizeof(kStateTable) / sizeof(kStateTable[0]);
}

[[nodiscard]] inline constexpr bool have_received_syn(State s) noexcept {
    return static_cast<unsigned char>(s) >= static_cast<unsigned char>(State::SynReceived);
}

[[nodiscard]] inline constexpr bool have_established(State s) noexcept {
    return static_cast<unsigned char>(s) >= static_cast<unsigned char>(State::Established);
}

[[nodiscard]] inline constexpr bool have_received_fin(State s) noexcept {
    return s == State::CloseWait
        || (static_cast<unsigned char>(s) >= static_cast<unsigned char>(State::Closing)
            && s != State::FinWait2);
}

[[nodiscard]] inline Status validate_state_transition(State from, State to) noexcept {
    if (from == State::Closed && to == State::Listen) {
        return Status::Ok;
    }
    if (from == State::Listen && to == State::SynReceived) {
        return Status::Ok;
    }
    if (from == State::SynSent && to == State::SynReceived) {
        return Status::Ok;
    }
    if (from == State::SynReceived && to == State::Established) {
        return Status::Ok;
    }
    if (from == State::Established && to == State::CloseWait) {
        return Status::Ok;
    }
    if (from == State::Established && to == State::FinWait1) {
        return Status::Ok;
    }
    if (from == to) {
        return Status::Ok;
    }
    return Status::Protocol;
}

[[nodiscard]] inline Status check_option(CapabilityRights sock_rights, Option opt,
                                         sockopt::OptDir dir) noexcept {
    CapabilityRights need = rights_for(opt, dir);
    if (!has_right(sock_rights, need)) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tcp
