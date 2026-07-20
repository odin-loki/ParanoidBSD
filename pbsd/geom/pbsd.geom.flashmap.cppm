module;
#include <cstdint>

export module pbsd.geom.flashmap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_flashmap.h — flashmap GEOM class.
export namespace pbsd::geom::flashmap {

inline constexpr unsigned kClassName = 0x464C5348; // "FLSH"

enum class Flag : unsigned int {
    ReadOnly  = 0x01,
    WriteOnly = 0x02,
    Erase     = 0x04,
};

struct Region {
    unsigned long long offset{};
    unsigned long long length{};
    unsigned           erase_size{};
};

[[nodiscard]] inline Status validate_region(Region const& r) noexcept {
    if (r.length == 0 || r.erase_size == 0) {
        return Status::Invalid;
    }
    if (r.offset % r.erase_size != 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::flashmap
