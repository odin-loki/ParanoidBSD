module;
#include <cstdint>

export module pbsd.stand.net;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/net.h — boot-time network I/O constants.
export namespace pbsd::stand::net {

inline constexpr unsigned kMaxPhys = 16;
inline constexpr unsigned kTimeout = 3000;

enum class Proto : unsigned char {
    None = 0,
    Arp  = 1,
    Rarp = 2,
    Bootp = 3,
    Tftp = 4,
};

[[nodiscard]] inline Status validate_proto(Proto p) noexcept {
    switch (p) {
    case Proto::Arp:
    case Proto::Rarp:
    case Proto::Bootp:
    case Proto::Tftp:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::stand::net
