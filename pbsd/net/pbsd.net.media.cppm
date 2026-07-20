module;
#include <cstdint>

export module pbsd.net.media;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_media.h — IFM_ETHER media subtypes.
export namespace pbsd::net::media {

inline constexpr unsigned kEther = 0x00000020;
inline constexpr unsigned kMaskType = 0x000000E0;

enum class Subtype : unsigned int {
    TenT      = 3,
    Ten2      = 4,
    Ten5      = 5,
    HundredTx = 6,
    HundredFx = 7,
    HundredT4 = 8,
    ThousandT = 16,
    TenG_Lr   = 18,
    TenG_Sr   = 19,
    TenG_T    = 26,
};

enum class Option : unsigned int {
    Fdx  = 0x00100000,
    Hdx  = 0x00200000,
    Flow = 0x00400000,
    Loop = 0x00800000,
};

[[nodiscard]] inline Status validate_subtype(Subtype s) noexcept {
    switch (s) {
    case Subtype::TenT:
    case Subtype::HundredTx:
    case Subtype::ThousandT:
    case Subtype::TenG_T:
        return Status::Ok;
    default:
        return Status::Ok;
    }
}

[[nodiscard]] inline constexpr bool option_has(unsigned o, Option bit) noexcept {
    return (o & static_cast<unsigned>(bit)) != 0;
}

} // namespace pbsd::net::media
