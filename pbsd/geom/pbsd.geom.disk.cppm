module;
#include <cstdint>

export module pbsd.geom.disk;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_disk.h — DISK rotation-rate constants.
export namespace pbsd::geom::disk {

inline constexpr unsigned kClassName = 0x4449534B; // "DISK"

enum class RotationRate : unsigned int {
    Unknown      = 0,
    NonRotating  = 1,
    Min          = 0x0401,
    Max          = 0xFFFE,
};

enum class InitLevel : unsigned char {
    None   = 0,
    Create = 1,
    Start  = 2,
    Done   = 3,
};

[[nodiscard]] inline Status validate_rotation_rate(unsigned rate) noexcept {
    if (rate == static_cast<unsigned>(RotationRate::Unknown)
        || rate == static_cast<unsigned>(RotationRate::NonRotating)) {
        return Status::Ok;
    }
    if (rate >= static_cast<unsigned>(RotationRate::Min)
        && rate <= static_cast<unsigned>(RotationRate::Max)) {
        return Status::Ok;
    }
    return Status::Invalid;
}

} // namespace pbsd::geom::disk
