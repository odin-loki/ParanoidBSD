module;
#include <cstdint>

export module pbsd.net.ip6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip6.h — IPv6 header and next-header constants.
export namespace pbsd::net::ip6 {

inline constexpr unsigned char kVersion = 6;
inline constexpr unsigned kAddrBytes = 16;
inline constexpr unsigned char kHopLimitDefault = 64;

enum class NextHeader : unsigned char {
    HopByHop = 0,
    Icmp6    = 58,
    None     = 59,
    DestOpts = 60,
    Routing  = 43,
    Fragment = 44,
    Udp      = 17,
    Tcp      = 6,
};

struct Header {
    unsigned char vfc{};
    unsigned char flow[3]{};
    unsigned short payload_len{};
    unsigned char nxt{};
    unsigned char hlim{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    unsigned ver = (h.vfc >> 4) & 0x0F;
    if (ver != kVersion) {
        return Status::Protocol;
    }
    if (h.hlim == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip6
