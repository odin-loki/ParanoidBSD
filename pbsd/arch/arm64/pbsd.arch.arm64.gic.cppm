module;
#include <cstdint>

export module pbsd.arch.arm64.gic;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/arm64/include/gic_v3.h — GICv3 IRQ types.
export namespace pbsd::arch::arm64::gic {

enum class IrqType : unsigned char {
    Spurious = 0,
    Ppi      = 1,
    Spi      = 2,
    Lpi      = 3,
};

enum class Trigger : unsigned char {
    Edge  = 0,
    Level = 1,
};

inline constexpr unsigned kMaxSpi = 1019;
inline constexpr unsigned kPpiBase = 16;

[[nodiscard]] inline Status validate_irq(unsigned irq, IrqType t) noexcept {
    if (t == IrqType::Spi && irq > kMaxSpi) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::arm64::gic
