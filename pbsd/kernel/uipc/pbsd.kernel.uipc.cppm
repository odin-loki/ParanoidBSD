module;
#include <cstdint>

export module pbsd.kernel.uipc;

export import pbsd.core;

/// Wave 4/5 — UNIX IPC/socket constants from sys/un.h, sys/socket.h, kern/uipc_*.c.
export namespace pbsd::kernel::uipc {

inline constexpr int kAfUnix = 1;
inline constexpr int kSockStream = 1;
inline constexpr int kSockDgram  = 2;
inline constexpr int kSockSeqpacket = 5;

inline constexpr unsigned kMsgOob     = 0x00000001;
inline constexpr unsigned kMsgPeek    = 0x00000002;
inline constexpr unsigned kMsgDontwait = 0x00000080;
inline constexpr unsigned kMsgEor     = 0x00000008;
inline constexpr unsigned kMsgTrunc   = 0x00000010;

inline constexpr unsigned kSoAcceptconn = 0x00000002;
inline constexpr unsigned kSoReuseaddr  = 0x00000004;
inline constexpr unsigned kSoKeepalive  = 0x00000008;
inline constexpr unsigned kSoDontroute  = 0x00000010;
inline constexpr unsigned kSoBroadcast  = 0x00000020;
inline constexpr unsigned kSoReuseport  = 0x00000200;

inline constexpr int kSolSocket = 0xffff;

inline constexpr int kSunPathMax = 104;

struct SockaddrUn {
    unsigned char sun_len{};
    unsigned char sun_family{};
    char          sun_path[kSunPathMax]{};
};

[[nodiscard]] constexpr Status validate_sock_type(int type) noexcept {
    switch (type) {
    case kSockStream:
    case kSockDgram:
    case kSockSeqpacket:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] constexpr Status validate_sun_path_len(unsigned len) noexcept {
    if (len < 2 || len > kSunPathMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool msg_flag_set(unsigned flags, unsigned bit) noexcept {
    return (flags & bit) != 0;
}

} // namespace pbsd::kernel::uipc
