module;
#include <cstdint>

export module pbsd.net.in_cksum;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_cksum.c — Internet checksum fold.
export namespace pbsd::net::in_cksum {

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffffu);
    sum += (sum >> 16);
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint16_t add16(std::uint16_t a, std::uint16_t b) noexcept {
    std::uint32_t sum = static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b);
    if (sum > 0xffffu) {
        sum = (sum & 0xffffu) + 1u;
    }
    return static_cast<std::uint16_t>(sum);
}

[[nodiscard]] inline std::uint32_t accumulate(std::uint32_t sum,
                                              std::uint16_t word) noexcept {
    return sum + static_cast<std::uint32_t>(word);
}

[[nodiscard]] inline std::uint16_t finish(std::uint32_t sum) noexcept {
    return fold32(sum);
}

} // namespace pbsd::net::in_cksum
