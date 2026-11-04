module;
#include <cstdint>

export module pbsd.uda.xae;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/xilinx/if_xae.c
export namespace pbsd::uda::xae {

inline constexpr RegInsn kInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor descriptor() noexcept {
    return Descriptor{
        .name = "xae",
        .provenance = "hbsd/src/sys/dev/xilinx/if_xae.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10EE,
        .device_id = 0x7028,
        .init_sequence = kInit,
        .reset_sequence = kInit,
    };
}

[[nodiscard]] inline Status validate_attach(bool attached) noexcept {
    return attached ? Status::Busy : Status::Ok;
}

} // namespace pbsd::uda::xae
