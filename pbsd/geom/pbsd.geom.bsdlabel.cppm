module;
#include <cstdint>

export module pbsd.geom.bsdlabel;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/bsd/g_bsd.c — BSD label provider geometry.
export namespace pbsd::geom::bsdlabel {

inline constexpr unsigned kSectorSize = 512;

struct Slice {
    unsigned long long offset{};
    unsigned long long length{};
    unsigned index{};
};

[[nodiscard]] inline Status validate_slice(const Slice& s) noexcept {
    if (s.length == 0) {
        return Status::Invalid;
    }
    if (s.offset % kSectorSize != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned long long end_offset(const Slice& s) noexcept {
    return s.offset + s.length;
}

} // namespace pbsd::geom::bsdlabel
