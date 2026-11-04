module;
#include <cstdint>

export module pbsd.kernel.socket_ops;

export import pbsd.core;
export import pbsd.kernel.filedesc;

/// Wave 4 — socket state/options (sys/socket.h, sys/socketvar.h).
export namespace pbsd::kernel::socket_ops {

using namespace pbsd::kernel::filedesc;

inline constexpr unsigned kSoDebug       = 0x00000001;
inline constexpr unsigned kSoAcceptconn  = 0x00000002;
inline constexpr unsigned kSoReuseaddr   = 0x00000004;
inline constexpr unsigned kSoKeepalive   = 0x00000008;
inline constexpr unsigned kSoDontroute   = 0x00000010;
inline constexpr unsigned kSoBroadcast   = 0x00000020;
inline constexpr unsigned kSoLinger      = 0x00000080;
inline constexpr unsigned kSoReuseport   = 0x00000200;
inline constexpr unsigned kSoTimestamp   = 0x00000400;
inline constexpr unsigned kSoNosigpipe   = 0x00000800;
inline constexpr unsigned kSoReuseportLb = 0x00010000;

inline constexpr int kSoSndbuf   = 0x1001;
inline constexpr int kSoRcvbuf   = 0x1002;
inline constexpr int kSoError    = 0x1007;
inline constexpr int kSoType     = 0x1008;

inline constexpr unsigned kSsIsconnected    = 0x0002;
inline constexpr unsigned kSsIsconnecting   = 0x0004;
inline constexpr unsigned kSsIsdisconnecting = 0x0008;
inline constexpr unsigned kSsNbio           = 0x0100;
inline constexpr unsigned kSsAsync          = 0x0200;
inline constexpr unsigned kSsIsdisconnected = 0x2000;

struct SocketStub {
    unsigned so_options{};
    short    so_state{};
    int      so_type{};
    int      so_error{};
};

struct OptionEntry {
    unsigned    option{};
    const char* name{};
};

inline constexpr OptionEntry kOptionTable[] = {
    {kSoReuseaddr,  "SO_REUSEADDR"},
    {kSoKeepalive,  "SO_KEEPALIVE"},
    {kSoBroadcast,  "SO_BROADCAST"},
    {kSoReuseport,  "SO_REUSEPORT"},
    {kSoNosigpipe,  "SO_NOSIGPIPE"},
    {kSoReuseportLb, "SO_REUSEPORT_LB"},
};

[[nodiscard]] inline unsigned option_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kOptionTable) / sizeof(kOptionTable[0]));
}

[[nodiscard]] constexpr bool has_so_option(unsigned opts, unsigned bit) noexcept {
    return (opts & bit) != 0;
}

[[nodiscard]] constexpr bool has_ss_state(unsigned short state, unsigned bit) noexcept {
    return (static_cast<unsigned>(state) & bit) != 0;
}

[[nodiscard]] constexpr bool so_listening(unsigned opts) noexcept {
    return has_so_option(opts, kSoAcceptconn);
}

[[nodiscard]] constexpr bool so_connected(unsigned short state) noexcept {
    return has_ss_state(state, kSsIsconnected);
}

[[nodiscard]] constexpr bool so_nonblocking(unsigned short state) noexcept {
    return has_ss_state(state, kSsNbio);
}

[[nodiscard]] constexpr Status validate_socket(const SocketStub& so) noexcept {
    if (so.so_type <= 0) {
        return Status::Invalid;
    }
    if (has_ss_state(so.so_state, kSsIsconnected)
        && has_ss_state(so.so_state, kSsIsdisconnected)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr int socket_dtype() noexcept {
    return kDtypeSocket;
}

[[nodiscard]] constexpr Status validate_sockopt_level(int level) noexcept {
    if (level == 0xFFFF || level == 0x0006 || level == 0x0011) {
        return Status::Ok;
    }
    return Status::Invalid;
}

} // namespace pbsd::kernel::socket_ops
