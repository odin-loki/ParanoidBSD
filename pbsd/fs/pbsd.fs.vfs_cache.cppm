module;
#include <cstdint>

export module pbsd.fs.vfs_cache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/vfs_cache.c — name cache sizing constants.
export namespace pbsd::fs::vfs_cache {

inline constexpr unsigned kDefaultBuckets = 256;
inline constexpr unsigned kMaxBuckets = 65536;

struct Stats {
    unsigned hits{};
    unsigned misses{};
    unsigned purged{};
};

[[nodiscard]] inline Status validate_buckets(unsigned buckets) noexcept {
    if (buckets == 0 || buckets > kMaxBuckets) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline void record_hit(Stats& s) noexcept {
    ++s.hits;
}

[[nodiscard]] inline void record_miss(Stats& s) noexcept {
    ++s.misses;
}

} // namespace pbsd::fs::vfs_cache
