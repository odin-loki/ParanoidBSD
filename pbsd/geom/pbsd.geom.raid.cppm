module;
#include <cstdint>

export module pbsd.geom.raid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/raid/g_raid.h — GEOM RAID device flags.
export namespace pbsd::geom::raid {

inline constexpr unsigned kVersion = 0;

enum class DeviceFlag : unsigned long long {
    NoAutosync = 0x0000000000000001ULL,
    NoFailsync = 0x0000000000000002ULL,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::raid
