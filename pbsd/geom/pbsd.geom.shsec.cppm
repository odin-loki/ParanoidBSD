module;
#include <cstdint>

export module pbsd.geom.shsec;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/shsec/g_shsec.h — GEOM SHSEC metadata version.
export namespace pbsd::geom::shsec {

inline constexpr unsigned kVersion = 1;

enum class BlockFlag : unsigned char {
    First = 0x1,
};

[[nodiscard]] inline Status validate_disk_count(unsigned count) noexcept {
    if (count < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::shsec
