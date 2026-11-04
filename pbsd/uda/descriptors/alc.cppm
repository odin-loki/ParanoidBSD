module;
#include <cstdint>

export module pbsd.uda.alc;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/alc/if_alc.c
export namespace pbsd::uda::alc {

inline constexpr std::uint32_t kRegMacCtl = 0x0050;
inline constexpr std::uint32_t kMacReset = 0x80000000u;

[[nodiscard]] inline Status validate_mii(unsigned reg) noexcept {
    return reg <= 31 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kAlcInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor alc_generic() noexcept {
    return Descriptor{
        .name = "alc",
        .provenance = "hbsd/src/sys/dev/alc/if_alc.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1969,
        .device_id = 0x1083,
        .init_sequence = kAlcInit,
        .reset_sequence = kAlcInit,
    };
}

} // namespace pbsd::uda::alc
