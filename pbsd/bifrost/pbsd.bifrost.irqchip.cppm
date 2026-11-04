module;
#include <cstdint>

export module pbsd.bifrost.irqchip;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vlapic.c
export namespace pbsd::bifrost::irqchip {

enum class DeliveryMode : unsigned char {
    Fixed = 0,
    Lowest = 1,
    Smi = 2,
    Nmi = 4,
    Init = 5,
    ExtInt = 7,
};

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec >= 16 && vec <= 255 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::bifrost::irqchip
