module;
#include <cstdint>

export module pbsd.kernel.in_cksum;

import pbsd.core;


/// Freestanding port of `netinet/in_cksum.c`.
export namespace pbsd::kernel::in_cksum {

inline constexpr unsigned kCsumOffset = 10;

[[nodiscard]] inline std::uint16_t fold32(std::uint32_t sum) noexcept {
    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;
    return static_cast<std::uint16_t>(~sum);
}

[[nodiscard]] inline std::uint32_t add16(std::uint32_t sum, std::uint16_t v) noexcept {
    return sum + v;
}

} // namespace pbsd::kernel::in_cksum
