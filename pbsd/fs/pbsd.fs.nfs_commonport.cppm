module;
#include <cstdint>

export module pbsd.fs.nfs_commonport;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nfs/nfs_commonport.c — NFS port mapper helpers.
export namespace pbsd::fs::nfs_commonport {

inline constexpr unsigned kPortMin = 1024;
inline constexpr unsigned kPortMax = 65535;
inline constexpr unsigned kDefaultPort = 2049;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    if (port < kPortMin || port > kPortMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned default_port() noexcept {
    return kDefaultPort;
}

struct PortBinding {
    unsigned port{kDefaultPort};
    bool tcp{true};
    bool udp{false};
};

[[nodiscard]] inline Status bind(PortBinding& b, unsigned port, bool tcp, bool udp) noexcept {
    if (validate_port(port) != Status::Ok) {
        return Status::Invalid;
    }
    if (!tcp && !udp) {
        return Status::Invalid;
    }
    b.port = port;
    b.tcp = tcp;
    b.udp = udp;
    return Status::Ok;
}

} // namespace pbsd::fs::nfs_commonport
