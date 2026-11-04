module;
#include <cstdint>

export module pbsd.arch.amd64.segments;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/x86/include/segments.h
export namespace pbsd::arch::amd64::segments {

inline constexpr unsigned kRplMask = 3;
inline constexpr unsigned kKernelPl = 0;
inline constexpr unsigned kUserPl = 3;
inline constexpr unsigned kLdtFlag = 4;

enum class DescType : unsigned char {
    SysNull = 0,
    Sys386Tss = 9,
    SysLdt = 2,
    Sys386CallGate = 12,
    Sys386IntGate = 14,
    Sys386TrapGate = 15,
    SysCode = 24,
    SysData = 26,
};

[[nodiscard]] inline std::uint16_t global_sel(unsigned index, unsigned rpl = kKernelPl) noexcept {
    return static_cast<std::uint16_t>((index << 3) | (rpl & kRplMask));
}

[[nodiscard]] inline unsigned index_from_sel(std::uint16_t sel) noexcept {
    return (sel >> 3) & 0x1FFFu;
}

[[nodiscard]] inline Status validate_selector(std::uint16_t sel) noexcept {
    return index_from_sel(sel) < 8192 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::segments
