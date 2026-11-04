module;
#include <cstdint>

export module pbsd.uda.iwi;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/iwi/if_iwi.c
export namespace pbsd::uda::iwi {

inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIwiInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iwi_generic() noexcept {
    return Descriptor{
        .name = "iwi",
        .provenance = "hbsd/src/sys/dev/iwi/if_iwi.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x4227,
        .init_sequence = kIwiInit,
        .reset_sequence = kIwiInit,
    };
}

} // namespace pbsd::uda::iwi
