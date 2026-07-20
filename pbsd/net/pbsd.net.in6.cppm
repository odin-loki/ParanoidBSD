module;
#include <cstdint>

export module pbsd.net.in6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/in6.h — IPv6 scope and option constants.
export namespace pbsd::net::in6 {

inline constexpr unsigned char kHopLimitDefault = 64;
inline constexpr unsigned char kVersion = 6;
inline constexpr unsigned kAddrBytes = 16;

enum class Scope : unsigned char {
    NodeLocal   = 0x01,
    LinkLocal   = 0x02,
    SiteLocal   = 0x05,
    OrgLocal    = 0x08,
    Global      = 0x0e,
};

enum class Option : unsigned char {
    HopLimit    = 1,
    TrafficClass = 2,
    HopOpts     = 3,
    Dstopts     = 4,
    Rthdr       = 5,
    Pktinfo     = 6,
    RthdrDstopts = 7,
    Recvpktinfo = 8,
    Recvhoplimit = 9,
    Recvtrafficclass = 10,
};

[[nodiscard]] inline Status validate_scope(Scope s) noexcept {
    switch (s) {
    case Scope::NodeLocal:
    case Scope::LinkLocal:
    case Scope::SiteLocal:
    case Scope::OrgLocal:
    case Scope::Global:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_hoplimit(unsigned char hl) noexcept {
    if (hl == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::in6
