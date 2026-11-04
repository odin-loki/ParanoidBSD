module;
#include <cstdint>

export module pbsd.uda.watchdog;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/watchdog/watchdog.c
export namespace pbsd::uda::watchdog {

inline constexpr RegInsn kInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor descriptor() noexcept {
    return Descriptor{
        .name = "watchdog",
        .provenance = "hbsd/src/sys/dev/watchdog/watchdog.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kInit,
        .reset_sequence = kInit,
    };
}

[[nodiscard]] inline Status validate_attach(bool attached) noexcept {
    return attached ? Status::Busy : Status::Ok;
}

} // namespace pbsd::uda::watchdog
