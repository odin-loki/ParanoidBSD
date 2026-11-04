module;
#include <cstdint>

export module pbsd.net.udp6_usrreq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/udp6_usrreq.c — UDP/IPv6 user-request helpers.
export namespace pbsd::net::udp6_usrreq {

enum class Req : unsigned char {
    Attach = 0,
    Bind = 1,
    Connect = 2,
    Disconnect = 3,
    Shutdown = 4,
};

struct PcbLite {
    std::uint16_t lport{};
    std::uint16_t fport{};
    bool attached{false};
    bool connected{false};
};

[[nodiscard]] inline Status attach(PcbLite& p) noexcept {
    if (p.attached) {
        return Status::Busy;
    }
    p.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status bind(PcbLite& p, std::uint16_t lport) noexcept {
    if (!p.attached || lport == 0) {
        return Status::Invalid;
    }
    p.lport = lport;
    return Status::Ok;
}

[[nodiscard]] inline Status connect_peer(PcbLite& p, std::uint16_t fport) noexcept {
    if (!p.attached || p.lport == 0 || fport == 0) {
        return Status::Invalid;
    }
    p.fport = fport;
    p.connected = true;
    return Status::Ok;
}

} // namespace pbsd::net::udp6_usrreq
