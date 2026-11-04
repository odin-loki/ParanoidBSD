module;
#include <cstdint>

export module pbsd.geom.subr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_subr.c — GEOM topology helper flags.
export namespace pbsd::geom::subr {

enum class TopoFlag : unsigned int {
    Orphan = 0x0001,
    Wither = 0x0002,
    Hardcoded = 0x0004,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    (void)flags;
    return Status::Ok;
}

[[nodiscard]] inline bool is_wither(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(TopoFlag::Wither)) != 0;
}

} // namespace pbsd::geom::subr
