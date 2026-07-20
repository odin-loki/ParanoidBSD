module;
#include <cstdint>

export module pbsd.net.sockopt;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/socket.h, sockopt.h — socket option tables.
export namespace pbsd::net::sockopt {

enum class Level : int {
    Socket = 0xffff, // SOL_SOCKET
    Tcp    = 6,      // IPPROTO_TCP
    Udp    = 17,     // IPPROTO_UDP
    Ip     = 0,      // IPPROTO_IP
};

/// Per-socket option flags (so_options bitmask).
enum class SoFlag : unsigned int {
    Debug        = 0x00000001,
    AcceptConn   = 0x00000002,
    ReuseAddr    = 0x00000004,
    KeepAlive    = 0x00000008,
    DontRoute    = 0x00000010,
    Broadcast    = 0x00000020,
    Linger       = 0x00000080,
    OobInline    = 0x00000100,
    ReusePort    = 0x00000200,
    Timestamp    = 0x00000400,
    NoSigpipe    = 0x00000800,
    NoOffload    = 0x00004000,
    ReusePortLb  = 0x00010000,
};

/// Additional socket options (sopt_name when level == SOL_SOCKET).
enum class SoName : unsigned int {
    SndBuf      = 0x1001,
    RcvBuf      = 0x1002,
    SndLowat    = 0x1003,
    RcvLowat    = 0x1004,
    SndTimeo    = 0x1005,
    RcvTimeo    = 0x1006,
    Error       = 0x1007,
    Type        = 0x1008,
    Fib         = 0x1014,
    Protocol    = 0x1016,
    Domain      = 0x1019,
    Splice      = 0x1023,
    Vendor      = 0x80000000,
};

enum class OptDir : unsigned char { Get = 0, Set = 1 }; // sopt_dir

struct OptionEntry {
    Level           level{};
    int             name{};
    CapabilityRights need_read{CapabilityRights::None};
    CapabilityRights need_write{CapabilityRights::None};
    bool            kernel_only{false};
};

/// Capability requirements at the socket boundary (maps cap_recv/cap_send checks).
[[nodiscard]] inline constexpr CapabilityRights rights_for(Level level, int name,
                                                           OptDir dir) noexcept {
    if (level == Level::Socket) {
        switch (static_cast<unsigned>(name)) {
        case static_cast<unsigned>(SoName::RcvBuf):
        case static_cast<unsigned>(SoName::RcvLowat):
        case static_cast<unsigned>(SoName::RcvTimeo):
        case static_cast<unsigned>(SoName::Error):
        case static_cast<unsigned>(SoName::Type):
        case static_cast<unsigned>(SoName::Domain):
        case static_cast<unsigned>(SoName::Protocol):
            return CapabilityRights::Read;
        case static_cast<unsigned>(SoName::SndBuf):
        case static_cast<unsigned>(SoName::SndLowat):
        case static_cast<unsigned>(SoName::SndTimeo):
        case static_cast<unsigned>(SoName::Fib):
            return dir == OptDir::Set ? (CapabilityRights::Read | CapabilityRights::Write)
                                      : CapabilityRights::Read;
        case static_cast<unsigned>(SoName::Splice):
            return dir == OptDir::Set ? (CapabilityRights::Read | CapabilityRights::Write)
                                      : CapabilityRights::Read;
        default:
            break;
        }
    }
    return dir == OptDir::Set ? CapabilityRights::Write : CapabilityRights::Read;
}

inline constexpr OptionEntry kSocketOptionTable[] = {
    {Level::Socket, static_cast<int>(SoName::SndBuf),
     CapabilityRights::Read, CapabilityRights::Write, false},
    {Level::Socket, static_cast<int>(SoName::RcvBuf),
     CapabilityRights::Read, CapabilityRights::Read, false},
    {Level::Socket, static_cast<int>(SoName::Splice),
     CapabilityRights::Read, CapabilityRights::Read | CapabilityRights::Write, false},
    {Level::Socket, static_cast<int>(SoName::Error),
     CapabilityRights::Read, CapabilityRights::None, false},
    {Level::Socket, static_cast<int>(SoName::Type),
     CapabilityRights::Read, CapabilityRights::None, false},
    {Level::Socket, static_cast<int>(SoName::Fib),
     CapabilityRights::Read, CapabilityRights::Write, false},
    {Level::Socket, static_cast<int>(SoName::Protocol),
     CapabilityRights::Read, CapabilityRights::None, false},
    {Level::Socket, static_cast<int>(SoName::Domain),
     CapabilityRights::Read, CapabilityRights::None, false},
};

[[nodiscard]] inline constexpr std::size_t option_table_size() noexcept {
    return sizeof(kSocketOptionTable) / sizeof(kSocketOptionTable[0]);
}

[[nodiscard]] inline Status check_socket_option(CapabilityRights sock_rights,
                                                Level level, int name,
                                                OptDir dir) noexcept {
    CapabilityRights need = rights_for(level, name, dir);
    if (!has_right(sock_rights, need)) {
        return Status::Denied;
    }
    return Status::Ok;
}

} // namespace pbsd::net::sockopt
