module;
#include <cstdint>

export module pbsd.net.epair;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_epair.c — epair(4) host/peer pairing flags.
export namespace pbsd::net::epair {

enum class Role : unsigned char {
    Host = 0,
    Peer = 1,
};

enum class Flag : unsigned int {
    Up       = 0x00000001,
    Running  = 0x00000040,
    Promisc  = 0x00000100,
    Monitor  = 0x00040000,
};

[[nodiscard]] inline Status validate_role(Role r) noexcept {
    switch (r) {
    case Role::Host:
    case Role::Peer:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool flag_has(unsigned f, Flag bit) noexcept {
    return (f & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::epair
