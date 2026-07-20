module;
#include <cstdint>

export module pbsd.net.clone;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_clone.h — cloned interface creation flags.
export namespace pbsd::net::clone {

enum class Flag : unsigned int {
    Autounit = 0x00000001,
    Needifaddr = 0x00000002,
    Maxunit = 0x00000004,
};

inline constexpr unsigned kMaxName = 16;

[[nodiscard]] inline Status validate_unit(unsigned unit) noexcept {
    if (unit == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kMaxName) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::clone
