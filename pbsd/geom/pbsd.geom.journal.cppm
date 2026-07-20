module;
#include <cstdint>

export module pbsd.geom.journal;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/journal/g_journal.h — GEOM JOURNAL metadata.
export namespace pbsd::geom::journal {

inline constexpr unsigned kVersion = 0;

enum class State : unsigned char {
    New = 0,
    Active = 1,
    Dirty = 2,
};

[[nodiscard]] inline Status validate_state(State s) noexcept {
    switch (s) {
    case State::New:
    case State::Active:
    case State::Dirty:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::geom::journal
