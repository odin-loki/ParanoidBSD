module;
#include <cstdint>

export module pbsd.net.tap;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_tap.h — TAP bridge device constants.
export namespace pbsd::net::tap {

inline constexpr unsigned kMruMax = 65535;

enum class VnetHdr : unsigned char {
    Disabled = 0,
    Enabled  = 1,
};

[[nodiscard]] inline Status validate_mru(unsigned mru) noexcept {
    if (mru == 0 || mru > kMruMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tap
