module;
#include <cstdint>

export module pbsd.geom.mountver;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/mountver/g_mountver.h — GEOM MOUNTVER constants.
export namespace pbsd::geom::mountver {

inline constexpr unsigned kVersion = 4;

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver != kVersion) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::mountver
