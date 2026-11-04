module;
#include <cstdint>

export module pbsd.net.ether;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/if_ether.h, net/ethernet.h — Ethernet constants.
export namespace pbsd::net::ether {

inline constexpr unsigned kAddrLen = 6;
inline constexpr unsigned kTypeIp  = 0x0800;
inline constexpr unsigned kTypeIpv6 = 0x86DD;
inline constexpr unsigned kTypeArp = 0x0806;

enum class Type : unsigned short {
    Ip   = kTypeIp,
    Ipv6 = kTypeIpv6,
    Arp  = kTypeArp,
};

struct Address {
    unsigned char octets[kAddrLen]{};
};

[[nodiscard]] inline bool is_broadcast(Address const& a) noexcept {
    for (unsigned i = 0; i < kAddrLen; ++i) {
        if (a.octets[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool is_multicast(Address const& a) noexcept {
    return (a.octets[0] & 0x01) != 0;
}

[[nodiscard]] inline Status validate_type(unsigned short t) noexcept {
    if (t != kTypeIp && t != kTypeIpv6 && t != kTypeArp) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ether
