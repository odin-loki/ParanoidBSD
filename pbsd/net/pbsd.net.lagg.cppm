module;
#include <cstdint>

export module pbsd.net.lagg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_lagg.h — LAGG protocol and port flags.
export namespace pbsd::net::lagg {

inline constexpr unsigned kMaxPorts = 32;
inline constexpr unsigned kMaxStacking = 4;

enum class Flag : unsigned int {
    HashL2 = 0x00000001,
    HashL3 = 0x00000002,
    HashL4 = 0x00000004,
};

enum class PortFlag : unsigned int {
    Slave        = 0x00000000,
    Master       = 0x00000001,
    Stack        = 0x00000002,
    Active       = 0x00000004,
    Collecting   = 0x00000008,
    Distributing = 0x00000010,
};

enum class Proto : unsigned char {
    None = 0,
    RoundRobin,
    Failover,
    LoadBalance,
    Lacp,
    Broadcast,
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    if (p == Proto::None) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_port_count(unsigned count) noexcept {
    if (count == 0 || count > kMaxPorts) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::lagg
