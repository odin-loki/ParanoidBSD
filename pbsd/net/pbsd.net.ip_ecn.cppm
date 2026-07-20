module;
#include <cstdint>

export module pbsd.net.ip_ecn;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_ecn.h — IPsec ECN tunnel semantics.
export namespace pbsd::net::ip_ecn {

inline constexpr int kAllowed   = 1;
inline constexpr int kForbidden = 0;
inline constexpr int kNoCare    = -1;

inline constexpr std::uint8_t kEcnMask = 0x03;

[[nodiscard]] inline Status validate_mode(int mode) noexcept {
    if (mode != kAllowed && mode != kForbidden && mode != kNoCare) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline void ingress(int mode, std::uint8_t& tos,
                                  std::uint8_t const& itos) noexcept {
    if (validate_mode(mode) != Status::Ok || mode == kNoCare) {
        return;
    }
    if (mode == kForbidden) {
        tos = static_cast<std::uint8_t>(tos & ~kEcnMask);
        return;
    }
    tos = static_cast<std::uint8_t>((tos & ~kEcnMask) | (itos & kEcnMask));
}

[[nodiscard]] inline Status egress(int mode, std::uint8_t const& tos,
                                   std::uint8_t& otos) noexcept {
    if (validate_mode(mode) != Status::Ok) {
        return Status::Invalid;
    }
    if (mode == kForbidden) {
        otos = static_cast<std::uint8_t>(tos & ~kEcnMask);
        return Status::Ok;
    }
    otos = tos;
    return Status::Ok;
}

} // namespace pbsd::net::ip_ecn
