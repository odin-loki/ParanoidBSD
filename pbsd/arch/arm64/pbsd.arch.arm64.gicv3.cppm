module;
#include <cstdint>

export module pbsd.arch.arm64.gicv3;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/arm64/gic_v3_reg.h — GICv3 distributor/CPU IF regs.
export namespace pbsd::arch::arm64::gicv3 {

inline constexpr unsigned kIrqNumMax = 1020;
inline constexpr unsigned kLastSpi = 1019;
inline constexpr unsigned kFirstLpi = 8192;

inline constexpr unsigned long long kPriorityMax = 0x00ull;
inline constexpr unsigned long long kPriorityMin = 0xFCull;

inline constexpr unsigned kGicdStatusr = 0x0010;
inline constexpr unsigned kGicdSetspiNsr = 0x0040;
inline constexpr unsigned kGicdClrspiNsr = 0x0048;
inline constexpr unsigned kGicdIrouterBase = 0x6000;

inline constexpr unsigned long long kGicdCtlrG1 = 1ull << 0;
inline constexpr unsigned long long kGicdCtlrG1a = 1ull << 1;
inline constexpr unsigned long long kGicdCtlrAreNs = 1ull << 4;
inline constexpr unsigned long long kGicdCtlrRwp = 1ull << 31;
inline constexpr unsigned long long kGicdIrouterIrm = 1ull << 31;
inline constexpr unsigned kGicdSpiIntidMask = 0x3ff;

enum class RegBlock : unsigned char {
    Dist = 0,
    Redist = 1,
    Its = 2,
};

[[nodiscard]] inline unsigned gicd_irouter(unsigned irq) noexcept {
    return kGicdIrouterBase + irq * 8;
}

[[nodiscard]] inline Status validate_spi(unsigned irq) noexcept {
    if (irq > kLastSpi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_lpi(unsigned irq) noexcept {
    if (irq < kFirstLpi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_priority(unsigned long long pri) noexcept {
    if (pri > kPriorityMin) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::gicv3
