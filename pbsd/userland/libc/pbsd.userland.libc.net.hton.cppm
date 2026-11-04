module;

#include <cstdint>

export module pbsd.userland.libc.net.hton;

/// htons/htonl/ntohs/ntohl from hbsd/src/lib/libc/net/htonl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint16_t htons(std::uint16_t host) noexcept {
    return static_cast<std::uint16_t>((host << 8) | (host >> 8));
}

[[nodiscard]] inline std::uint16_t ntohs(std::uint16_t net) noexcept { return htons(net); }

[[nodiscard]] inline std::uint32_t htonl(std::uint32_t host) noexcept {
    return ((host & 0x000000FFU) << 24) | ((host & 0x0000FF00U) << 8) |
           ((host & 0x00FF0000U) >> 8) | ((host & 0xFF000000U) >> 24);
}

[[nodiscard]] inline std::uint32_t ntohl(std::uint32_t net) noexcept { return htonl(net); }

} // namespace pbsd::userland::libc
