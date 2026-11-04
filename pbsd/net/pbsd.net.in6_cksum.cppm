module;
#include <cstdint>

export module pbsd.net.in6_cksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/in6_cksum.c — IPv6 pseudo-header checksum.
export namespace pbsd::net::in6_cksum {

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffffu);
    sum += (sum >> 16);
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint32_t pseudo_hdr(std::uint32_t sum,
                                              std::uint32_t len,
                                              std::uint8_t nxt) noexcept {
    sum += (len >> 16) & 0xffffu;
    sum += len & 0xffffu;
    sum += static_cast<std::uint32_t>(nxt);
    return sum;
}

[[nodiscard]] inline std::uint16_t finish(std::uint32_t sum) noexcept {
    return fold32(sum);
}

} // namespace pbsd::net::in6_cksum
