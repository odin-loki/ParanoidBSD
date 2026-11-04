module;
#include <cstdint>

export module pbsd.net.in_rmx;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_rmx.c — IPv4 route prefix flag helpers.
export namespace pbsd::net::in_rmx {

enum class PfxFlag : unsigned int {
    Onlink   = 0x00000001,
    Broadcast = 0x00000002,
    Network  = 0x00000004,
};

struct PrefixInfo {
    unsigned int flags{};
    unsigned int plen{};
    bool         is_broadcast{};
};

[[nodiscard]] inline Status validate_plen(unsigned int plen) noexcept {
    if (plen > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status set_nhop_flags(PrefixInfo& info, unsigned int plen,
                                         bool is_broadcast) noexcept {
    if (validate_plen(plen) != Status::Ok) {
        return Status::Invalid;
    }
    info.plen = plen;
    info.is_broadcast = is_broadcast;
    info.flags = static_cast<unsigned int>(PfxFlag::Onlink);
    if (is_broadcast) {
        info.flags |= static_cast<unsigned int>(PfxFlag::Broadcast);
    } else if (plen < 32) {
        info.flags |= static_cast<unsigned int>(PfxFlag::Network);
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_onlink(unsigned int flags) noexcept {
    return (flags & static_cast<unsigned int>(PfxFlag::Onlink)) != 0;
}

} // namespace pbsd::net::in_rmx
