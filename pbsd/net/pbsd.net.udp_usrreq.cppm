module;
#include <cstdint>

export module pbsd.net.udp_usrreq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/udp_usrreq.c — UDP PCB user-request helpers.
export namespace pbsd::net::udp_usrreq {

enum class Req : unsigned char {
    Attach = 0,
    Bind = 1,
    Connect = 2,
    Disconnect = 3,
    Send = 4,
    Abort = 5,
};

struct PcbLite {
    std::uint16_t lport{};
    std::uint16_t fport{};
    bool attached{false};
};

[[nodiscard]] inline Status attach(PcbLite& p) noexcept {
    if (p.attached) {
        return Status::Busy;
    }
    p.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status bind_port(PcbLite& p, std::uint16_t port) noexcept {
    if (!p.attached || port == 0) {
        return Status::Invalid;
    }
    p.lport = port;
    return Status::Ok;
}

[[nodiscard]] inline Status connect_peer(PcbLite& p, std::uint16_t fport) noexcept {
    if (!p.attached || p.lport == 0 || fport == 0) {
        return Status::Invalid;
    }
    p.fport = fport;
    return Status::Ok;
}

} // namespace pbsd::net::udp_usrreq
