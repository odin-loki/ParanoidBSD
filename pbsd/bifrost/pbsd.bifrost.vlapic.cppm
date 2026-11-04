module;
#include <cstdint>

export module pbsd.bifrost.vlapic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/io/vlapic_priv.h — virtual LAPIC register offsets.
export namespace pbsd::bifrost::vlapic {

inline constexpr unsigned kOffsetId = 0x20;
inline constexpr unsigned kOffsetVer = 0x30;
inline constexpr unsigned kOffsetTpr = 0x80;
inline constexpr unsigned kOffsetEoi = 0xB0;
inline constexpr unsigned kOffsetSvr = 0xF0;
inline constexpr unsigned kOffsetIcrLow = 0x300;
inline constexpr unsigned kOffsetIcrHi = 0x310;
inline constexpr unsigned kOffsetTimerLvt = 0x320;

[[nodiscard]] inline Status validate_offset(unsigned offset) noexcept {
    if (offset > 0x400) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_vector(int vector) noexcept {
    if (vector < 16 || vector > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::vlapic
