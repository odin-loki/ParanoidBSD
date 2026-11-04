module;
#include <cstdint>

export module pbsd.net.in_mcast;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_mcast.h — IPv4 multicast group limits.
export namespace pbsd::net::in_mcast {

inline constexpr unsigned kMaxGroups = 4096;
inline constexpr unsigned kLoopDefault = 1;

struct Group {
    std::uint32_t addr{};
    unsigned short ifindex{};
};

[[nodiscard]] inline Status validate_group(const Group& g) noexcept {
    if (g.addr < 0xe0000000u || g.addr >= 0xf0000000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_count(unsigned count) noexcept {
    if (count > kMaxGroups) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::in_mcast
