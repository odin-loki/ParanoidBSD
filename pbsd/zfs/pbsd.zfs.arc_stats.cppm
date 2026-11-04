module;
#include <cstdint>

export module pbsd.zfs.arc_stats;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/arc.c — ARC stat counters.
export namespace pbsd::zfs::arc_stats {

struct Counters {
    std::uint64_t hits{};
    std::uint64_t misses{};
    std::uint64_t evictions{};
    std::uint64_t size{};
};

[[nodiscard]] inline Status record_hit(Counters& c) noexcept {
    ++c.hits;
    return Status::Ok;
}

[[nodiscard]] inline Status record_miss(Counters& c) noexcept {
    ++c.misses;
    return Status::Ok;
}

[[nodiscard]] inline Status record_eviction(Counters& c, std::uint64_t bytes) noexcept {
    ++c.evictions;
    if (c.size < bytes) {
        return Status::Invalid;
    }
    c.size -= bytes;
    return Status::Ok;
}

[[nodiscard]] inline double hit_ratio(const Counters& c) noexcept {
    const auto total = c.hits + c.misses;
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(c.hits) / static_cast<double>(total);
}

} // namespace pbsd::zfs::arc_stats
