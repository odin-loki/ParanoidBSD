module;
#include <cstdint>

export module pbsd.stand.kmod;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/module.h — kmod loader flags.
export namespace pbsd::stand::kmod {

enum class Flag : unsigned int {
    Glob    = 0x01,
    NoUnload = 0x02,
    Loader  = 0x04,
};

enum class Event : unsigned char {
    Load   = 0,
    Unload = 1,
    Shutdown = 2,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::NoUnload))
        && (flags & static_cast<unsigned>(Flag::Loader)) == 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::kmod
