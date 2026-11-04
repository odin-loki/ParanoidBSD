module;
#include <cstdint>

export module pbsd.net.in_proto;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_proto.c — IPv4 protocol switch entries.
export namespace pbsd::net::in_proto {

enum class Proto : unsigned char {
    Icmp = 1,
    Igmp = 2,
    Tcp  = 6,
    Udp  = 17,
    Raw  = 255,
};

struct Entry {
    Proto proto{Proto::Tcp};
    bool reachable{true};
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    switch (p) {
    case Proto::Icmp:
    case Proto::Igmp:
    case Proto::Tcp:
    case Proto::Udp:
    case Proto::Raw:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status dispatch(const Entry& e) noexcept {
    if (!e.reachable) {
        return Status::Denied;
    }
    return validate_proto(e.proto);
}

} // namespace pbsd::net::in_proto
