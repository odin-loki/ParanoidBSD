module;
#include <cstdint>

export module pbsd.net.ip_id;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_id.c — IPv4 ID generation.
export namespace pbsd::net::ip_id {

struct Generator {
    std::uint16_t next{1};
    std::uint16_t wrap{65535};
};

[[nodiscard]] inline std::uint16_t next_id(Generator& g) noexcept {
    const std::uint16_t id = g.next;
    if (g.next >= g.wrap) {
        g.next = 1;
    } else {
        ++g.next;
    }
    return id;
}

[[nodiscard]] inline Status validate_range(std::uint16_t lo, std::uint16_t hi) noexcept {
    if (lo == 0 || hi == 0 || lo > hi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::ip_id
