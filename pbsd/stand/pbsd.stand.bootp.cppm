module;
#include <cstdint>

export module pbsd.stand.bootp;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/bootp.c, bootp.h — BOOTP/DHCP boot constants.
export namespace pbsd::stand::bootp {

inline constexpr unsigned kPortServer = 67;
inline constexpr unsigned kPortClient = 68;
inline constexpr unsigned kVendSize = 312;

enum class Op : unsigned char {
    Request = 1,
    Reply = 2,
};

enum class Tag : unsigned char {
    Pad = 0,
    SubnetMask = 1,
    Gateway = 3,
    Hostname = 12,
    BootSize = 13,
    DhcpMsgType = 53,
    ServerId = 54,
    End = 255,
};

enum class DhcpMsg : unsigned char {
    Discover = 1,
    Offer = 2,
    Request = 3,
    Decline = 4,
    Ack = 5,
    Nak = 6,
    Release = 7,
};

[[nodiscard]] inline Status validate_op(Op op) noexcept {
    return op == Op::Request || op == Op::Reply ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port == kPortServer || port == kPortClient ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::bootp
