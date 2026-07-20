module;
#include <cstdint>

export module pbsd.net.vxlan;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_vxlan.h — VXLAN header and port constants.
export namespace pbsd::net::vxlan {

inline constexpr unsigned kPort = 4789;
inline constexpr unsigned kLegacyPort = 8472;
inline constexpr unsigned kVniMax = 1u << 24;
inline constexpr unsigned kVniMask = kVniMax - 1;
inline constexpr unsigned kHdrValidVni = 0x08000000;
inline constexpr unsigned kVniShift = 8;

struct Header {
    unsigned vxlh_flags{};
    unsigned vxlh_vni{};
};

[[nodiscard]] inline Status validate_vni(unsigned vni) noexcept {
    if (vni >= kVniMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_header(Header const& h) noexcept {
    if ((h.vxlh_flags & kHdrValidVni) == 0) {
        return Status::Protocol;
    }
    return validate_vni((h.vxlh_vni >> kVniShift) & kVniMask);
}

} // namespace pbsd::net::vxlan
