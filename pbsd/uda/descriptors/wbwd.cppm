module;
#include <cstdint>

export module pbsd.uda.wbwd;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/wbwd/wbwd.c
export namespace pbsd::uda::wbwd {

inline constexpr RegInsn kInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor descriptor() noexcept {
    return Descriptor{
        .name = "wbwd",
        .provenance = "hbsd/src/sys/dev/wbwd/wbwd.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x8086,
        .device_id = 0x0000,
        .init_sequence = kInit,
        .reset_sequence = kInit,
    };
}

[[nodiscard]] inline Status validate_attach(bool attached) noexcept {
    return attached ? Status::Busy : Status::Ok;
}

} // namespace pbsd::uda::wbwd
