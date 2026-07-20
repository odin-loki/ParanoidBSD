module;
#include <cstdint>

export module pbsd.net.gre;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/gre.h — GRE protocol and flag bits.
export namespace pbsd::net::gre {

inline constexpr unsigned short kEtherType = 0x6558;
inline constexpr unsigned short kProtoIp = 0x0800;
inline constexpr unsigned short kProtoIp6 = 0x86DD;

enum class Flag : unsigned short {
    Checksum = 0x8000,
    Routing  = 0x4000,
    Key      = 0x2000,
    Seq      = 0x1000,
    Strict   = 0x0800,
    Recursion = 0x0700,
    Ack      = 0x0080,
};

struct Header {
    unsigned short flags_version{};
    unsigned short proto{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    unsigned ver = h.flags_version & 0x0007;
    if (ver != 0 && ver != 1) {
        return Status::Protocol;
    }
    if (h.proto == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(unsigned short f, Flag bit) noexcept {
    return (f & static_cast<unsigned short>(bit)) != 0;
}

} // namespace pbsd::net::gre
