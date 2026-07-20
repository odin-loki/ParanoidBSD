module;
#include <cstdint>

export module pbsd.arch.arm64.smmu;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/iommu/smmureg.h
export namespace pbsd::arch::arm64::smmu {

inline constexpr unsigned kIdr0 = 0x000;
inline constexpr unsigned kIdr1 = 0x004;
inline constexpr unsigned kCr0 = 0x020;
inline constexpr unsigned kCr1 = 0x024;

inline constexpr std::uint32_t kIdr0StLvlLinear = 0x0u << 27;
inline constexpr std::uint32_t kIdr0StLvl2 = 0x1u << 27;
inline constexpr std::uint32_t kIdr0Vmid16 = 1u << 18;
inline constexpr std::uint32_t kIdr0TtendianLittle = 0x2u << 21;

inline constexpr std::uint32_t kCr0SmmuEn = 1u << 0;
inline constexpr std::uint32_t kCr0TlbDis = 1u << 10;

[[nodiscard]] inline Status validate_idr0(std::uint32_t idr0) noexcept {
    (void)idr0;
    return Status::Ok;
}

[[nodiscard]] inline bool smmu_enabled(std::uint32_t cr0) noexcept {
    return (cr0 & kCr0SmmuEn) != 0;
}

} // namespace pbsd::arch::arm64::smmu
