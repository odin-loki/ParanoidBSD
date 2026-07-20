module;
#include <cstdint>

export module pbsd.uda.iicbus;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/iicbus/iicbus.c
export namespace pbsd::uda::iicbus {

inline constexpr unsigned kIicMaxAddr = 127;

[[nodiscard]] inline Status validate_addr(unsigned addr) noexcept {
    return addr <= kIicMaxAddr ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIicbusInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iicbus_generic() noexcept {
    return Descriptor{
        .name = "iicbus",
        .provenance = "hbsd/src/sys/dev/iicbus/iicbus.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kIicbusInit,
        .reset_sequence = kIicbusInit,
    };
}

} // namespace pbsd::uda::iicbus
