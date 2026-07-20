module;
#include <cstdint>

export module pbsd.uda.hwpmc;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hwpmc/hwpmc.c
export namespace pbsd::uda::hwpmc {

inline constexpr unsigned kPmcMaxEvents = 32;

[[nodiscard]] inline Status validate_event(unsigned ev) noexcept {
    return ev < kPmcMaxEvents ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kHwpmcInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hwpmc_generic() noexcept {
    return Descriptor{
        .name = "hwpmc",
        .provenance = "hbsd/src/sys/dev/hwpmc/hwpmc.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kHwpmcInit,
        .reset_sequence = kHwpmcInit,
    };
}

} // namespace pbsd::uda::hwpmc
