module;
#include <cstddef>

export module pbsd.stand.preload;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/preload.c
export namespace pbsd::stand::preload {

inline constexpr unsigned kMaxModules = 16;
inline constexpr unsigned kPathLen = 256;

[[nodiscard]] inline Status validate_module_count(unsigned count) noexcept {
    return count <= kMaxModules ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::preload
