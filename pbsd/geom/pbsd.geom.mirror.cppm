module;
#include <cstdint>

export module pbsd.geom.mirror;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/mirror/g_mirror.h — MIRROR class metadata.
export namespace pbsd::geom::mirror {

inline constexpr char kClassName[] = "MIRROR";
inline constexpr char kMagic[] = "GEOM::MIRROR";
inline constexpr unsigned kVersion = 4;

enum class Balance : unsigned char {
    None       = 0,
    RoundRobin = 1,
    Load       = 2,
    Prefer     = 3,
};

enum class Flag : unsigned int {
    NoFailureSync = 0x01,
    NoSync        = 0x02,
    Destroy       = 0x04,
};

[[nodiscard]] inline Status validate_balance(Balance b) noexcept {
    if (static_cast<unsigned char>(b) > static_cast<unsigned char>(Balance::Prefer)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::mirror
