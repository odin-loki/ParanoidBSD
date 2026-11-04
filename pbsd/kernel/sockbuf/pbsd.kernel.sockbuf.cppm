module;
#include <cstdint>

export module pbsd.kernel.sockbuf;

export import pbsd.core;

/// Wave 4 — socket buffer limits (sys/sys/socketvar.h, kern/uipc_sockbuf.c).
export namespace pbsd::kernel::sockbuf {

inline constexpr unsigned kSsIsconnected    = 0x0002;
inline constexpr unsigned kSsIsconnecting   = 0x0004;
inline constexpr unsigned kSsIsdisconnecting = 0x0008;
inline constexpr unsigned kSsNbio           = 0x0100;
inline constexpr unsigned kSsAsync          = 0x0200;
inline constexpr unsigned kSsIsdisconnected = 0x2000;

inline constexpr unsigned kDefaultSendSpace = 9216;
inline constexpr unsigned kDefaultRecvSpace = 87380;
inline constexpr unsigned kMaxSockbuf       = 512 * 1024;

struct SockbufStub {
    unsigned sb_cc{};
    unsigned sb_hiwat{};
    unsigned sb_lowat{};
    unsigned so_state{};
};

[[nodiscard]] constexpr bool so_connected(unsigned state) noexcept {
    return (state & kSsIsconnected) != 0;
}

[[nodiscard]] constexpr bool so_connecting(unsigned state) noexcept {
    return (state & kSsIsconnecting) != 0;
}

[[nodiscard]] constexpr Status validate_hiwat(unsigned hi) noexcept {
    if (hi == 0 || hi > kMaxSockbuf) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_lowat(unsigned lo, unsigned hi) noexcept {
    if (lo > hi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_sockbuf(const SockbufStub& sb) noexcept {
    if (validate_hiwat(sb.sb_hiwat) != Status::Ok) {
        return Status::Invalid;
    }
    if (validate_lowat(sb.sb_lowat, sb.sb_hiwat) != Status::Ok) {
        return Status::Invalid;
    }
    if (sb.sb_cc > sb.sb_hiwat) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool space_available(const SockbufStub& sb) noexcept {
    return sb.sb_cc < sb.sb_hiwat;
}

} // namespace pbsd::kernel::sockbuf
