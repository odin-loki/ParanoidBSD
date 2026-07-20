module;
#include <cstdint>

export module pbsd.net.ipdivert;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_divert.c — divert socket port range helpers.
export namespace pbsd::net::ipdivert {

inline constexpr unsigned kPortMin = 1;
inline constexpr unsigned kPortMax = 65535;
inline constexpr unsigned kDefaultPort = 8668;

enum class Dir : unsigned char {
    In = 0,
    Out = 1,
};

struct Softc {
    unsigned port{kDefaultPort};
    unsigned packets{0};
    unsigned bytes{0};
    bool active{false};
};

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    if (port < kPortMin || port > kPortMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status activate(Softc& sc, unsigned port) noexcept {
    if (validate_port(port) != Status::Ok) {
        return Status::Invalid;
    }
    sc.port = port;
    sc.active = true;
    return Status::Ok;
}

[[nodiscard]] inline Status account(Softc& sc, unsigned nbytes, Dir /*dir*/) noexcept {
    if (!sc.active) {
        return Status::Denied;
    }
    ++sc.packets;
    sc.bytes += nbytes;
    return Status::Ok;
}

} // namespace pbsd::net::ipdivert
