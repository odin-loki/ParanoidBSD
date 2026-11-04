module;
#include <cstdint>

export module pbsd.net.carp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_carp.h — CARP/VRRP advertisement constants.
export namespace pbsd::net::carp {

inline constexpr unsigned char kDefaultTtl = 255;
inline constexpr unsigned char kDefaultInterval = 1;
inline constexpr unsigned char kAdvertisement = 0x01;
inline constexpr unsigned kKeyLen = 20;
inline constexpr unsigned kVrrpMaxInterval = 0x1000 - 1;

struct Header {
    unsigned char version_type{};
    unsigned char vhid{};
    unsigned char advskew{};
    unsigned char authlen{};
    unsigned char advbase{};
    unsigned short cksum{};
};

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if (h.vhid == 0) {
        return Status::Invalid;
    }
    if (h.advbase == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::carp
