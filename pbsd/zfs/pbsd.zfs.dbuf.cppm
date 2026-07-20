module;
#include <cstdint>

export module pbsd.zfs.dbuf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/dbuf.h — dbuf states.
export namespace pbsd::zfs::dbuf {

enum class State : signed char {
    Marker    = -2,
    Search    = -1,
    Uncached  = 0,
    Fill      = 1,
    Nofill    = 2,
    Read      = 3,
    Cached    = 4,
    Evicting  = 5,
};

enum class CacheSlot : signed char {
    NoCache         = -1,
    DbufCache       = 0,
    DbufMetadata    = 1,
};

[[nodiscard]] inline bool is_valid_state(State s) noexcept {
    return static_cast<signed char>(s) >= static_cast<signed char>(State::Uncached)
        && static_cast<signed char>(s) <= static_cast<signed char>(State::Evicting);
}

[[nodiscard]] inline bool is_cached(State s) noexcept {
    return s == State::Cached;
}

[[nodiscard]] inline bool is_evictable(State s) noexcept {
    return s == State::Cached || s == State::Uncached;
}

[[nodiscard]] inline Status validate_transition(State from, State to) noexcept {
    if (!is_valid_state(from) || !is_valid_state(to)) {
        return Status::Invalid;
    }
    if (from == State::Search || to == State::Search) {
        return Status::Invalid;
    }
    if (from == State::Evicting && to != State::Uncached) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cache_slot(CacheSlot slot) noexcept {
    if (static_cast<signed char>(slot) < static_cast<signed char>(CacheSlot::DbufCache)
        || static_cast<signed char>(slot) > static_cast<signed char>(CacheSlot::DbufMetadata)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dbuf
