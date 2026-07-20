module;
#include <cstddef>

export module pbsd.stand.lua;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/lua/load.lua
export namespace pbsd::stand::lua {

inline constexpr unsigned kMaxModules = 8;
inline constexpr unsigned kScriptLen = 4096;

[[nodiscard]] inline Status validate_module_count(unsigned count) noexcept {
    return count <= kMaxModules ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::stand::lua
