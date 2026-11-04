module;
#include <cstdint>

export module pbsd.uda.bwi;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bwi/if_bwi.c
export namespace pbsd::uda::bwi {

inline constexpr std::uint32_t kRegMacCtl = 0x0400;

[[nodiscard]] inline Status validate_rate(unsigned rate) noexcept {
    return rate <= 54 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kBwiInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bwi_generic() noexcept {
    return Descriptor{
        .name = "bwi",
        .provenance = "hbsd/src/sys/dev/bwi/if_bwi.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x168c,
        .device_id = 0x0013,
        .init_sequence = kBwiInit,
        .reset_sequence = kBwiInit,
    };
}

} // namespace pbsd::uda::bwi
