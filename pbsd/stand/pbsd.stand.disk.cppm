module;
#include <cstdint>

export module pbsd.stand.disk;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/disk.c — loader disk open/partition constants.
export namespace pbsd::stand::disk {

inline constexpr unsigned kDefaultSectorSize = 512;
inline constexpr unsigned kMaxSectorSize = 4096;
inline constexpr unsigned kMaxPartitions = 128;

enum class OpenMode : unsigned char {
    Read = 0,
    Write = 1,
};

struct OpenHint {
    unsigned partition{0};
    unsigned sectorsize{kDefaultSectorSize};
    OpenMode mode{OpenMode::Read};
};

[[nodiscard]] inline Status validate_sectorsize(unsigned size) noexcept {
    if (size < kDefaultSectorSize || size > kMaxSectorSize) {
        return Status::Invalid;
    }
    if ((size & (size - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_open(const OpenHint& hint) noexcept {
    if (hint.partition > kMaxPartitions) {
        return Status::Invalid;
    }
    return validate_sectorsize(hint.sectorsize);
}

} // namespace pbsd::stand::disk
