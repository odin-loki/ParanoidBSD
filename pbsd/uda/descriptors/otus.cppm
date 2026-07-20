module;
#include <cstdint>

export module pbsd.uda.otus;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/otus/if_otus.c
export namespace pbsd::uda::otus {

inline constexpr std::uint32_t kRegMacCtl = 0x0000;

[[nodiscard]] inline Status validate_band(unsigned band) noexcept {
    return band < 3 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kOtusInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor otus_generic() noexcept {
    return Descriptor{
        .name = "otus",
        .provenance = "hbsd/src/sys/dev/otus/if_otus.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0cf3,
        .device_id = 0x1002,
        .init_sequence = kOtusInit,
        .reset_sequence = kOtusInit,
    };
}

} // namespace pbsd::uda::otus
