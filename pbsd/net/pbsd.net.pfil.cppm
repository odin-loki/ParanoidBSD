module;
#include <cstdint>

export module pbsd.net.pfil;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfil.h — packet filter hook types and flags.
export namespace pbsd::net::pfil {

inline constexpr unsigned kMaxName = 64;

enum class Type : unsigned char {
    Ip4 = 0,
    Ip6 = 1,
    Ethernet = 2,
};

enum class Flag : unsigned int {
    In  = 0x00010000,
    Out = 0x00020000,
    Fwd = 0x00040000,
};

[[nodiscard]] inline Status validate_type(Type t) noexcept {
    switch (t) {
    case Type::Ip4:
    case Type::Ip6:
    case Type::Ethernet:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline constexpr bool flag_has(unsigned f, Flag bit) noexcept {
    return (f & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::pfil
