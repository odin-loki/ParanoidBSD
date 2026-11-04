module;
#include <cstdint>

export module pbsd.net.tcp_subr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_subr.c — TCP helper constants.
export namespace pbsd::net::tcp_subr {

inline constexpr unsigned kDefaultMss = 536;
inline constexpr unsigned kMaxMss = 65495;
inline constexpr unsigned kMinMss = 216;

[[nodiscard]] inline Status validate_mss(unsigned mss) noexcept {
    if (mss < kMinMss || mss > kMaxMss) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned clamp_mss(unsigned mss) noexcept {
    if (mss < kMinMss) {
        return kMinMss;
    }
    if (mss > kMaxMss) {
        return kMaxMss;
    }
    return mss;
}

struct IsnParams {
    std::uint32_t secret{};
    std::uint32_t counter{};
};

[[nodiscard]] inline std::uint32_t next_isn(IsnParams& p) noexcept {
    ++p.counter;
    return p.secret ^ p.counter;
}

} // namespace pbsd::net::tcp_subr
