module;
#include <cstdint>

export module pbsd.net.ether_ifattach;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_ethersubr.c — Ethernet ifattach flags.
export namespace pbsd::net::ether_ifattach {

enum class Flag : unsigned {
    Broadcast = 0x0001,
    Multicast = 0x0002,
    Promisc   = 0x0004,
};

struct Attach {
    unsigned short ifindex{};
    unsigned flags{};
    unsigned char mac[6]{};
};

[[nodiscard]] inline Status validate_mac(const unsigned char mac[6]) noexcept {
    bool all_zero = true;
    for (unsigned i = 0; i < 6; ++i) {
        if (mac[i] != 0) {
            all_zero = false;
            break;
        }
    }
    return all_zero ? Status::Invalid : Status::Ok;
}

[[nodiscard]] inline Status validate_attach(const Attach& a) noexcept {
    if (a.ifindex == 0) {
        return Status::Invalid;
    }
    return validate_mac(a.mac);
}

} // namespace pbsd::net::ether_ifattach
