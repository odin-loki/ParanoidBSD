module;
#include <cstdint>

export module pbsd.net.gif;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_gif.h — GIF tunnel MTU and option flags.
export namespace pbsd::net::gif {

inline constexpr unsigned kMtuDefault = 1280;
inline constexpr unsigned kMtuMin = 1280;
inline constexpr unsigned kMtuMax = 8192;
inline constexpr unsigned kEtheripVersion = 0x3;

enum class Opt : unsigned int {
    NoClamp       = 0x0001,
    IgnoreSource  = 0x0002,
};

[[nodiscard]] inline Status validate_mtu(unsigned mtu) noexcept {
    if (mtu < kMtuMin || mtu > kMtuMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool opt_has(unsigned o, Opt bit) noexcept {
    return (o & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::gif
