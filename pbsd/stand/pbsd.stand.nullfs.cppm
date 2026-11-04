module;
#include <cstddef>

export module pbsd.stand.nullfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/nullfs.c
export namespace pbsd::stand::nullfs {

enum class MountMode : unsigned char {
    Passthrough = 0,
    Blackhole = 1,
};

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    return mode <= static_cast<unsigned>(MountMode::Blackhole) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::nullfs
