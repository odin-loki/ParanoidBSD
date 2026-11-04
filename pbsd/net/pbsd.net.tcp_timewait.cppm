module;
#include <cstdint>

export module pbsd.net.tcp_timewait;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_timer.h — 2MSL timewait bucket sizing.
export namespace pbsd::net::tcp_timewait {

inline constexpr unsigned kBucketCount = 32;
inline constexpr unsigned kMslSeconds = 30;
inline constexpr unsigned k2MslMs = kMslSeconds * 2 * 1000;

struct Bucket {
    unsigned entries{};
};

[[nodiscard]] inline Status validate_bucket(unsigned idx) noexcept {
    if (idx >= kBucketCount) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash) noexcept {
    return hash % kBucketCount;
}

[[nodiscard]] inline Status enter(Bucket& b) noexcept {
    ++b.entries;
    return Status::Ok;
}

[[nodiscard]] inline Status leave(Bucket& b) noexcept {
    if (b.entries == 0) {
        return Status::Invalid;
    }
    --b.entries;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_timewait
