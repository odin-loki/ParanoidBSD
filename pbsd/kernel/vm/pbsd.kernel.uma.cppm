module;
#include <cstdint>

export module pbsd.kernel.uma;

import pbsd.core;

/// Freestanding port of `uma.h` zone flags + sizing policy.
export namespace pbsd::kernel::uma {

inline constexpr unsigned kUmaSmallestUnit = 8u;

inline constexpr unsigned kZoneUnmanaged   = 0x0001u;
inline constexpr unsigned kZoneZinit      = 0x0002u;
inline constexpr unsigned kZoneContig     = 0x0004u;
inline constexpr unsigned kZoneNotouch    = 0x0008u;
inline constexpr unsigned kZoneMalloc     = 0x0010u;
inline constexpr unsigned kZoneNofree     = 0x0020u;
inline constexpr unsigned kZoneVm         = 0x0080u;
inline constexpr unsigned kZoneNotpage    = 0x0100u;
inline constexpr unsigned kZoneSecondary  = 0x0200u;
inline constexpr unsigned kZoneNobucket   = 0x0400u;
inline constexpr unsigned kZoneMaxbucket  = 0x0800u;
inline constexpr unsigned kZoneNotrim     = 0x1000u;
inline constexpr unsigned kZoneCachespread = 0x2000u;
inline constexpr unsigned kZoneNodump     = 0x4000u;
inline constexpr unsigned kZonePcpu       = 0x8000u;

struct ZoneParams {
    unsigned item_size{};
    unsigned flags{};
    unsigned max_items{};
    unsigned cur_items{};
};

[[nodiscard]] constexpr unsigned align_item_size(unsigned size) noexcept {
    if (size < kUmaSmallestUnit) {
        return kUmaSmallestUnit;
    }
    unsigned a = kUmaSmallestUnit;
    while (a < size) {
        a <<= 1;
    }
    return a;
}

[[nodiscard]] constexpr Status validate_zone_flags(unsigned flags) noexcept {
    if ((flags & kZoneMalloc) != 0 && (flags & kZoneVm) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// `uma_zalloc_arg` — cap on zone growth (sysctl-backed in full kernel).
[[nodiscard]] inline Status zone_alloc_allowed(const ZoneParams& z,
                                               unsigned request) noexcept {
    if (request == 0) {
        return Status::Invalid;
    }
    if (z.max_items != 0 && z.cur_items + request > z.max_items) {
        return Status::NoMemory;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool zone_zeroes(unsigned flags) noexcept {
    return (flags & kZoneZinit) != 0;
}

} // namespace pbsd::kernel::uma
