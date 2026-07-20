module;
#include <cstdint>

export module pbsd.net.rss;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_rss.c — Receive-side scaling bucket map.
export namespace pbsd::net::rss {

inline constexpr unsigned kMaxBuckets = 128;
inline constexpr unsigned kDefaultBuckets = 8;

struct Table {
    unsigned buckets{kDefaultBuckets};
    unsigned cpu_map[kMaxBuckets]{};
};

[[nodiscard]] inline Status configure(Table& t, unsigned buckets) noexcept {
    if (buckets == 0 || buckets > kMaxBuckets) {
        return Status::Invalid;
    }
    t.buckets = buckets;
    for (unsigned i = 0; i < buckets; ++i) {
        t.cpu_map[i] = i;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status map_hash(const Table& t, std::uint32_t hash,
                                     unsigned& cpu_out) noexcept {
    if (t.buckets == 0 || t.buckets > kMaxBuckets) {
        return Status::Invalid;
    }
    cpu_out = t.cpu_map[hash % t.buckets];
    return Status::Ok;
}

} // namespace pbsd::net::rss
