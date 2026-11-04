module;
#include <cstdint>

export module pbsd.geom.multipath;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/multipath/g_multipath.h — GEOM MULTIPATH metadata.
export namespace pbsd::geom::multipath {

inline constexpr unsigned kVersion = 1;

enum class State : unsigned char {
    New = 0,
    Active = 1,
    Failed = 2,
};

[[nodiscard]] inline Status validate_provider_count(unsigned count) noexcept {
    if (count < 2) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Failed:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::multipath
