module;
#include <cstdint>

export module pbsd.net.ip_encap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_encap.h — IP encapsulation types.
export namespace pbsd::net::ip_encap {

enum class Type : unsigned char {
    Direct = 0,
    Gre    = 1,
    Mobile = 2,
    Ipsec  = 3,
    Gif    = 4,
};

struct Entry {
    Type type{Type::Direct};
    unsigned short proto{};
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Direct:
    case Type::Gre:
    case Type::Mobile:
    case Type::Ipsec:
    case Type::Gif:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (validate_type(e.type) != Status::Ok) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip_encap
