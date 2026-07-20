module;
#include <cstdint>

export module pbsd.net.tcp_usrreq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_usrreq.c — TCP PCB user-request helpers.
export namespace pbsd::net::tcp_usrreq {

enum class Req : unsigned char {
    Attach = 0,
    Bind = 1,
    Listen = 2,
    Connect = 3,
    Disconnect = 4,
    Accept = 5,
    Shutdown = 6,
    Abort = 7,
};

struct PcbLite {
    std::uint16_t lport{};
    std::uint16_t fport{};
    bool attached{false};
    bool listening{false};
};

[[nodiscard]] inline Status attach(PcbLite& p) noexcept {
    if (p.attached) {
        return Status::Busy;
    }
    p.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status listen(PcbLite& p, std::uint16_t lport) noexcept {
    if (!p.attached || lport == 0) {
        return Status::Invalid;
    }
    p.lport = lport;
    p.listening = true;
    return Status::Ok;
}

[[nodiscard]] inline Status connect_peer(PcbLite& p, std::uint16_t fport) noexcept {
    if (!p.attached || p.lport == 0 || fport == 0 || p.listening) {
        return Status::Invalid;
    }
    p.fport = fport;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_usrreq
