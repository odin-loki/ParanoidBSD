module;
#include <cstdint>

export module pbsd.geom.virstor;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/virstor/g_virstor.h — GEOM VIRSTOR metadata.
export namespace pbsd::geom::virstor {

inline constexpr unsigned kVersion = 1;

enum class State : unsigned char {
    New    = 0,
    Active = 1,
    Full   = 2,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Full:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_chunk_size(unsigned bytes) noexcept {
    if (bytes < 512 || (bytes % 512) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::virstor
