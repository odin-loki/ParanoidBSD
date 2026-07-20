module;
#include <cstdint>

export module pbsd.net.bridge;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_bridgevar.h — bridge ioctl flags.
export namespace pbsd::net::bridge {

enum class Flag : unsigned int {
    Learning  = 0x0001,
    Discover  = 0x0002,
    Stp       = 0x0004,
    StpUser   = 0x0008,
    StpAuto   = 0x0010,
    StpP2p    = 0x0020,
    Private   = 0x0040,
    Age       = 0x0080,
    Ieee8021d = 0x0100,
};

enum class Span : unsigned char {
    Disabled = 0,
    Enabled  = 1,
    Auto     = 2,
};

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if ((flags & static_cast<unsigned>(Flag::StpUser))
        && (flags & static_cast<unsigned>(Flag::StpAuto))) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::bridge
