module;
#include <cstdint>

export module pbsd.geom.gunion;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/union/g_union.h — GEOM UNION overlay.
export namespace pbsd::geom::gunion {

inline constexpr char kClassName[] = "UNION";
inline constexpr unsigned kVersion = 1;
inline constexpr char kSuffix[] = ".union";
inline constexpr char kPhyspathPassthrough[] = "\255";

enum class Flag : unsigned {
    DoingCommit = 0x00000001,
};

struct Softc {
    unsigned long long map_size{};
    long root_size{};
    long leaf_size{};
    long bits_per_leaf{};
    long long offset{};
    long long size{};
    long long sector_size{};
    long flags{};
    long reads{};
    long writes{};
};

[[nodiscard]] inline Status validate_geometry(long long offset, long long size,
                                              long long sector_size) noexcept {
    if (sector_size <= 0 || size <= 0 || offset < 0) {
        return Status::Invalid;
    }
    if (size % sector_size != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool commit_in_progress(long flags) noexcept {
    return (flags & static_cast<long>(Flag::DoingCommit)) != 0;
}

[[nodiscard]] inline Status try_commit_lock(Softc& sc) noexcept {
    if (commit_in_progress(sc.flags)) {
        return Status::Busy;
    }
    sc.flags |= static_cast<long>(Flag::DoingCommit);
    return Status::Ok;
}

[[nodiscard]] inline Status release_commit_lock(Softc& sc) noexcept {
    sc.flags &= ~static_cast<long>(Flag::DoingCommit);
    return Status::Ok;
}

inline void account_read(Softc& sc, long bytes) noexcept {
    ++sc.reads;
    (void)bytes;
}

inline void account_write(Softc& sc, long bytes) noexcept {
    ++sc.writes;
    (void)bytes;
}

} // namespace pbsd::geom::gunion
