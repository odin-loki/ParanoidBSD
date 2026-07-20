module;
#include <cstdint>

export module pbsd.geom.raid3;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/raid3/g_raid3.h — GEOM RAID3 metadata version.
export namespace pbsd::geom::raid3 {

inline constexpr unsigned kVersion = 5;

enum class DiskFlag : unsigned long long {
    Dirty         = 0x0000000000000001ULL,
    Synchronizing = 0x0000000000000002ULL,
};

[[nodiscard]] inline Status validate_width(unsigned width) noexcept {
    if (width < 3) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::raid3
