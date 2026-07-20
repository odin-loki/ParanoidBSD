module;
#include <cstdint>

export module pbsd.uda.liquidio;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/liquidio/lio_main.c
export namespace pbsd::uda::liquidio {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kLiquidioInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor liquidio_generic() noexcept {
    return Descriptor{
        .name = "liquidio",
        .provenance = "hbsd/src/sys/dev/liquidio/lio_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x177d,
        .device_id = 0x9702,
        .init_sequence = kLiquidioInit,
        .reset_sequence = kLiquidioInit,
    };
}

} // namespace pbsd::uda::liquidio
