module;
#include <cstdint>

export module pbsd.zfs.zfetch;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dmu_zfetch.h — zfetch streams.
export namespace pbsd::zfs::zfetch {

inline constexpr unsigned kRanges = 9;

struct Range {
    unsigned short start{};
    unsigned short end{};
};

struct Stream {
    unsigned long long blkid{};
    unsigned atime{};
    Range ranges[kRanges]{};
    unsigned pf_dist{};
    unsigned ipf_dist{};
    unsigned long long pf_start{};
    unsigned long long pf_end{};
    bool missed{};
    bool more{};
};

struct State {
    int numstreams{};
    unsigned callers{};
};

[[nodiscard]] inline Status validate_range(Range const& r) noexcept {
    if (r.end < r.start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init_stream(Stream& s, unsigned long long blkid) noexcept {
    s.blkid = blkid;
    s.atime = 0;
    s.pf_dist = 0;
    s.ipf_dist = 0;
    s.pf_start = blkid;
    s.pf_end = blkid;
    s.missed = false;
    s.more = false;
    for (unsigned i = 0; i < kRanges; ++i) {
        s.ranges[i] = {};
    }
    return Status::Ok;
}

[[nodiscard]] inline Status add_range(Stream& s, unsigned index, Range const& r) noexcept {
    if (index >= kRanges) {
        return Status::Invalid;
    }
    if (validate_range(r) != Status::Ok) {
        return Status::Invalid;
    }
    s.ranges[index] = r;
    return Status::Ok;
}

[[nodiscard]] inline Status note_access(Stream& s, unsigned long long blkid) noexcept {
    if (blkid < s.blkid) {
        s.missed = true;
    }
    s.blkid = blkid;
    ++s.atime;
    return Status::Ok;
}

[[nodiscard]] inline bool needs_more_prefetch(Stream const& s) noexcept {
    return s.more || s.missed;
}

} // namespace pbsd::zfs::zfetch
