module;
#include <cstdint>

export module pbsd.net.ip6_mroute;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/ip6_mroute.h — IPv6 multicast routing.
export namespace pbsd::net::ip6_mroute {

inline constexpr int kMrt6Done      = 101;
inline constexpr int kMrt6AddMif    = 102;
inline constexpr int kMrt6DelMif    = 103;
inline constexpr int kMrt6AddMfc    = 104;
inline constexpr int kMrt6DelMfc    = 105;
inline constexpr int kMrt6Pim       = 107;
inline constexpr int kMrt6Init      = 108;

inline constexpr unsigned kMaxMifs = 64;
inline constexpr unsigned kIfSetSize = 256;
inline constexpr unsigned kBitsPerMask = 32;

using Mifi = unsigned short;
using IfMask = unsigned;

struct IfSet {
    IfMask bits[(kIfSetSize + kBitsPerMask - 1) / kBitsPerMask]{};
};

[[nodiscard]] inline void if_set(Mifi n, IfSet& set) noexcept {
    if (n < kIfSetSize) {
        set.bits[n / kBitsPerMask] |= (1u << (n % kBitsPerMask));
    }
}

[[nodiscard]] inline void if_clr(Mifi n, IfSet& set) noexcept {
    if (n < kIfSetSize) {
        set.bits[n / kBitsPerMask] &= ~(1u << (n % kBitsPerMask));
    }
}

[[nodiscard]] inline bool if_isset(Mifi n, IfSet const& set) noexcept {
    if (n >= kIfSetSize) {
        return false;
    }
    return (set.bits[n / kBitsPerMask] & (1u << (n % kBitsPerMask))) != 0;
}

[[nodiscard]] inline Status validate_mif(Mifi mif) noexcept {
    if (mif >= kMaxMifs) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_sockopt(int cmd) noexcept {
    switch (cmd) {
    case kMrt6Done:
    case kMrt6AddMif:
    case kMrt6DelMif:
    case kMrt6AddMfc:
    case kMrt6DelMfc:
    case kMrt6Pim:
    case kMrt6Init:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::ip6_mroute
