module;
#include <cstdint>

export module pbsd.net.loop;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if.h — loopback interface flags.
export namespace pbsd::net::loopback {

inline constexpr unsigned kIffLoopback = 0x8;
inline constexpr unsigned kMtuDefault = 16384;

enum class Flag : unsigned int {
    Up       = 0x1,
    Loopback = 0x8,
    Running  = 0x40,
};

[[nodiscard]] inline constexpr bool is_loopback(unsigned flags) noexcept {
    return (flags & kIffLoopback) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (!is_loopback(flags)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::loopback
