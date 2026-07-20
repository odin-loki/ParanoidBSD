module;
#include <cstdint>

export module pbsd.net.in6_pcb;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/in6_pcb.c — IPv6 PCB user-request helpers.
export namespace pbsd::net::in6_pcb {

enum class Req : unsigned char {
    Attach = 0,
    Bind = 1,
    Connect = 2,
    Disconnect = 3,
    Listen = 4,
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

[[nodiscard]] inline Status bind(PcbLite& p, std::uint16_t lport) noexcept {
    if (!p.attached || lport == 0) {
        return Status::Invalid;
    }
    p.lport = lport;
    return Status::Ok;
}

[[nodiscard]] inline Status listen(PcbLite& p, std::uint16_t lport) noexcept {
    if (bind(p, lport) != Status::Ok) {
        return Status::Invalid;
    }
    p.listening = true;
    return Status::Ok;
}

} // namespace pbsd::net::in6_pcb
