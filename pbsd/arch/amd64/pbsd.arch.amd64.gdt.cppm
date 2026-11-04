module;
#include <cstdint>

export module pbsd.arch.amd64.gdt;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/amd64/machdep.c — GDT selector layout.
export namespace pbsd::arch::amd64::gdt {

inline constexpr unsigned kNullSel = 0;
inline constexpr unsigned kKernelCodeSel = 1;
inline constexpr unsigned kKernelDataSel = 2;
inline constexpr unsigned kUserCode32Sel = 3;
inline constexpr unsigned kUserDataSel = 4;
inline constexpr unsigned kUserCode64Sel = 5;
inline constexpr unsigned kTssSel = 6;
inline constexpr unsigned kNumEntries = 8;

inline constexpr std::uint16_t selector(unsigned index, unsigned rpl = 0) noexcept {
    return static_cast<std::uint16_t>((index << 3) | (rpl & 3u));
}

[[nodiscard]] inline Status validate_index(unsigned index) noexcept {
    return index < kNumEntries ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool is_user_sel(std::uint16_t sel) noexcept {
    return (sel & 3u) == 3u;
}

} // namespace pbsd::arch::amd64::gdt
