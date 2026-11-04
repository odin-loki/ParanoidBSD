module;
#include <cstdint>

export module pbsd.geom.taste;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom.h — G_TF_* taste flags.
export namespace pbsd::geom::taste {

enum class Flag : unsigned int {
    Normal      = 0,
    Insist      = 1,
    Transparent = 2,
};

[[nodiscard]] inline Status validate_flag(Flag f) noexcept {
    if (static_cast<unsigned>(f) > static_cast<unsigned>(Flag::Transparent)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool is_transparent(Flag f) noexcept {
    return f == Flag::Transparent;
}

} // namespace pbsd::geom::taste
