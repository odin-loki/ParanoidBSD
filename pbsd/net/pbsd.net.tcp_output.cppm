module;
#include <cstdint>

export module pbsd.net.tcp_output;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_output.c — TCP output segment sizing.
export namespace pbsd::net::tcp_output {

inline constexpr unsigned kMinMss = 536;
inline constexpr unsigned kMaxMss = 65535;
inline constexpr unsigned kDefaultMss = 1460;

struct Segment {
    unsigned seq{};
    unsigned len{};
    unsigned mss{kDefaultMss};
    bool push{false};
};

[[nodiscard]] inline Status validate_mss(unsigned mss) noexcept {
    if (mss < kMinMss || mss > kMaxMss) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_segment(const Segment& seg) noexcept {
    if (validate_mss(seg.mss) != Status::Ok) {
        return Status::Invalid;
    }
    if (seg.len > seg.mss) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned payload_room(unsigned mss, unsigned hdr_len) noexcept {
    if (mss <= hdr_len) {
        return 0;
    }
    return mss - hdr_len;
}

} // namespace pbsd::net::tcp_output
