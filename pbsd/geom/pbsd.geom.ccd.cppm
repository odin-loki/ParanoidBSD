module;
#include <cstdint>

export module pbsd.geom.ccd;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_ccd.c — concatenated CCD unit planner.
export namespace pbsd::geom::ccd {

inline constexpr unsigned kMaxComponents = 16;

struct Component {
    std::uint64_t sectors{};
    std::uint32_t sector_size{512};
};

struct Unit {
    Component comps[kMaxComponents]{};
    unsigned count{};
    bool interleaved{false};
};

[[nodiscard]] inline Status add_component(Unit& u, Component c) noexcept {
    if (c.sectors == 0 || c.sector_size == 0) {
        return Status::Invalid;
    }
    if (u.count >= kMaxComponents) {
        return Status::Busy;
    }
    u.comps[u.count++] = c;
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t total_sectors(const Unit& u) noexcept {
    std::uint64_t sum = 0;
    for (unsigned i = 0; i < u.count; ++i) {
        sum += u.comps[i].sectors;
    }
    return sum;
}

[[nodiscard]] inline Status validate(const Unit& u) noexcept {
    if (u.count == 0) {
        return Status::Invalid;
    }
    const auto ss = u.comps[0].sector_size;
    for (unsigned i = 1; i < u.count; ++i) {
        if (u.comps[i].sector_size != ss) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::geom::ccd
