module;
#include <cstdint>

export module pbsd.uda.cxgbe;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cxgbe/t4_main.c
export namespace pbsd::uda::cxgbe {

inline constexpr std::uint32_t kRegCtrl = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCxgbeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cxgbe_generic() noexcept {
    return Descriptor{
        .name = "cxgbe",
        .provenance = "hbsd/src/sys/dev/cxgbe/t4_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1425,
        .device_id = 0x4400,
        .init_sequence = kCxgbeInit,
        .reset_sequence = kCxgbeInit,
    };
}

} // namespace pbsd::uda::cxgbe
