module;
#include <cstdint>

export module pbsd.net.ip6_id;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/ip6_id.c — IPv6 fragment ID generation.
export namespace pbsd::net::ip6_id {

struct Generator {
    std::uint32_t next{1};
    std::uint32_t wrap{0xffffffffu};
};

[[nodiscard]] inline std::uint32_t next_id(Generator& g) noexcept {
    const std::uint32_t id = g.next;
    if (g.next >= g.wrap) {
        g.next = 1;
    } else {
        ++g.next;
    }
    return id;
}

} // namespace pbsd::net::ip6_id
