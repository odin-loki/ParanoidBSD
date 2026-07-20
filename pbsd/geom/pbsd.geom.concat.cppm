module;
#include <cstdint>

export module pbsd.geom.concat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/concat/g_concat.h — GEOM_CONCAT constants.
export namespace pbsd::geom::concat {

inline constexpr unsigned kClassName = 0x434F4E43; // "CONC"

enum class State : unsigned char {
    New    = 0,
    Active = 1,
    Failed = 2,
};

[[nodiscard]] inline Status validate_provider_count(unsigned count) noexcept {
    if (count == 0) {
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

} // namespace pbsd::geom::concat
