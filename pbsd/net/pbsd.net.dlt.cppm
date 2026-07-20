module;
#include <cstdint>

export module pbsd.net.dlt;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/dlt.h — libpcap link-layer header types.
export namespace pbsd::net::dlt {

inline constexpr unsigned kLowMatchingMin = 0;

enum class Type : unsigned int {
    Null    = 0,
    En10Mb  = 1,
    En3Mb   = 2,
    Ax25    = 3,
    Pronet  = 4,
    Chaos   = 5,
    Ieee802 = 6,
    Arcnet  = 7,
    Slip    = 8,
    Ppp     = 9,
    Fddi    = 10,
};

[[nodiscard]] inline Status validate_type(unsigned t) noexcept {
    if (t > static_cast<unsigned>(Type::Fddi)) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::net::dlt
