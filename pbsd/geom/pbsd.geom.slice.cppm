module;
#include <cstdint>

export module pbsd.geom.slice;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_slice.h — GEOM slice geometry.
export namespace pbsd::geom::slice {

struct Slice {
    long long offset{};
    long long length{};
    unsigned  sector_size{};
};

struct HotSlice {
    long long offset{};
    long long length{};
    int       read_active{};
    int       delete_active{};
    int       write_active{};
};

[[nodiscard]] inline Status validate_slice(Slice const& s) noexcept {
    if (s.length <= 0 || s.sector_size == 0) {
        return Status::Invalid;
    }
    if (s.offset < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline long long end_offset(Slice const& s) noexcept {
    return s.offset + s.length;
}

} // namespace pbsd::geom::slice
