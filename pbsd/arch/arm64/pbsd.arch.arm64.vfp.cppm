module;
#include <cstdint>

export module pbsd.arch.arm64.vfp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/vfp.h
export namespace pbsd::arch::arm64::vfp {

inline constexpr std::uint32_t kFpcrAhp = 0x0400'0000u;
inline constexpr std::uint32_t kFpcrDn = 0x0200'0000u;
inline constexpr std::uint32_t kFpcrFz = 0x0100'0000u;
inline constexpr std::uint32_t kFpcrInit = 0;

inline constexpr std::uint32_t kRmodeMask = 0x00C0'0000u;
inline constexpr std::uint32_t kRmodeNearest = 0x0000'0000u;
inline constexpr std::uint32_t kRmodePi = 0x0040'0000u;
inline constexpr std::uint32_t kRmodeNi = 0x0080'0000u;
inline constexpr std::uint32_t kRmodeZero = 0x00C0'0000u;

[[nodiscard]] inline Status validate_fpcr(std::uint32_t fpcr) noexcept {
    if ((fpcr & ~0x07FF'FFFFu) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline std::uint32_t rounding_mode(std::uint32_t fpcr) noexcept {
    return fpcr & kRmodeMask;
}

} // namespace pbsd::arch::arm64::vfp
