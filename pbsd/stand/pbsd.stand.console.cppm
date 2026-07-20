module;
#include <cstdint>

export module pbsd.stand.console;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/console.c
export namespace pbsd::stand::console {

inline constexpr unsigned kDefaultCols = 80;
inline constexpr unsigned kDefaultRows = 25;

struct Geometry {
    unsigned cols{kDefaultCols};
    unsigned rows{kDefaultRows};
};

[[nodiscard]] inline Status validate_geometry(const Geometry& g) noexcept {
    if (g.cols == 0 || g.rows == 0 || g.cols > 512 || g.rows > 512) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::console
