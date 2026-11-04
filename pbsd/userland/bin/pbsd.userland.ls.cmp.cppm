module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.ls.cmp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ls/cmp.c — FTS sort comparators (logic-only).
export namespace pbsd::userland::bin::ls::cmp {

struct TimeSpec {
    std::int64_t sec{0};
    std::int64_t nsec{0};
};

struct StatSnap {
    TimeSpec mtime{};
    TimeSpec atime{};
    TimeSpec birthtime{};
    TimeSpec ctime{};
    std::int64_t size{0};
};

struct Entry {
    const char* name{nullptr};
    StatSnap stat{};
};

[[nodiscard]] inline int compare_time(const TimeSpec& a, const TimeSpec& b) noexcept {
    if (b.sec > a.sec) {
        return 1;
    }
    if (b.sec < a.sec) {
        return -1;
    }
    if (b.nsec > a.nsec) {
        return 1;
    }
    if (b.nsec < a.nsec) {
        return -1;
    }
    return 0;
}

[[nodiscard]] inline int namecmp(const Entry& a, const Entry& b) noexcept {
    return hosted::cstrcmp(a.name, b.name);
}

[[nodiscard]] inline int revnamecmp(const Entry& a, const Entry& b) noexcept {
    return hosted::cstrcmp(b.name, a.name);
}

[[nodiscard]] inline int modcmp(const Entry& a, const Entry& b,
                                bool same_sort) noexcept {
    const int t = compare_time(a.stat.mtime, b.stat.mtime);
    if (t != 0) {
        return t;
    }
    return same_sort ? revnamecmp(a, b) : namecmp(a, b);
}

[[nodiscard]] inline int revmodcmp(const Entry& a, const Entry& b,
                                   bool same_sort) noexcept {
    return modcmp(b, a, same_sort);
}

[[nodiscard]] inline int acccmp(const Entry& a, const Entry& b,
                                bool same_sort) noexcept {
    const int t = compare_time(a.stat.atime, b.stat.atime);
    if (t != 0) {
        return t;
    }
    return same_sort ? revnamecmp(a, b) : namecmp(a, b);
}

[[nodiscard]] inline int sizecmp(const Entry& a, const Entry& b) noexcept {
    if (a.stat.size < b.stat.size) {
        return -1;
    }
    if (a.stat.size > b.stat.size) {
        return 1;
    }
    return namecmp(a, b);
}

[[nodiscard]] inline int revsizecmp(const Entry& a, const Entry& b) noexcept {
    return sizecmp(b, a);
}

} // namespace pbsd::userland::bin::ls::cmp
