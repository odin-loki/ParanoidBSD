module;
#include <cstdint>

export module pbsd.uda.ice;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ice/if_ice_iflib.c
export namespace pbsd::uda::ice {

inline constexpr std::uint32_t kRegCtrl = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIceInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ice_generic() noexcept {
    return Descriptor{
        .name = "ice",
        .provenance = "hbsd/src/sys/dev/ice/if_ice_iflib.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x1591,
        .init_sequence = kIceInit,
        .reset_sequence = kIceInit,
    };
}

} // namespace pbsd::uda::ice
