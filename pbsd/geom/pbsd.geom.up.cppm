module;
#include <cstdint>

export module pbsd.geom.up;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_up.c — GEOM orphan/up provider tracking.
export namespace pbsd::geom::up {

inline constexpr unsigned kMaxUp = 256;

struct UpEntry {
    const char* name{};
    unsigned refs{};
    bool orphan{false};
};

[[nodiscard]] inline Status register_up(UpEntry& e, const char* name) noexcept {
    if (name == nullptr) {
        return Status::Invalid;
    }
    e.name = name;
    e.refs = 1;
    e.orphan = false;
    return Status::Ok;
}

[[nodiscard]] inline Status orphan(UpEntry& e) noexcept {
    if (e.refs == 0) {
        return Status::Invalid;
    }
    e.orphan = true;
    return Status::Ok;
}

[[nodiscard]] inline Status release(UpEntry& e) noexcept {
    if (e.refs == 0) {
        return Status::Invalid;
    }
    --e.refs;
    return e.refs == 0 ? Status::Ok : Status::Busy;
}

} // namespace pbsd::geom::up
