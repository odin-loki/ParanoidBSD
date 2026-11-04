module;
#include <cstdint>

export module pbsd.net.tcp_ecn;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_ecn.h — TCP ECN flag helpers.
export namespace pbsd::net::tcp_ecn {

inline constexpr std::uint8_t kThEce = 0x40;
inline constexpr std::uint8_t kThCwr = 0x80;
inline constexpr unsigned kAceShift = 6;

enum class Codepoint : std::uint8_t {
    NotEct = 0,
    Ect1   = 1,
    Ect0   = 2,
    Ce     = 3,
};

[[nodiscard]] inline Status validate_codepoint(Codepoint cp) noexcept {
    return static_cast<unsigned>(cp) <= static_cast<unsigned>(Codepoint::Ce)
        ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline std::uint8_t tos_codepoint(std::uint8_t tos) noexcept {
    return static_cast<std::uint8_t>(tos & 0x03);
}

[[nodiscard]] inline bool is_ce(std::uint8_t tos) noexcept {
    return tos_codepoint(tos) == static_cast<std::uint8_t>(Codepoint::Ce);
}

[[nodiscard]] inline std::uint8_t merge_flags(std::uint8_t flags, bool ece, bool cwr) noexcept {
    auto out = static_cast<std::uint8_t>(flags & ~(kThEce | kThCwr));
    if (ece) {
        out = static_cast<std::uint8_t>(out | kThEce);
    }
    if (cwr) {
        out = static_cast<std::uint8_t>(out | kThCwr);
    }
    return out;
}

} // namespace pbsd::net::tcp_ecn
