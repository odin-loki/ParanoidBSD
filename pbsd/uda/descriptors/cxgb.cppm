module;
#include <cstdint>

export module pbsd.uda.cxgb;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cxgb/cxgb_main.c
export namespace pbsd::uda::cxgb {

inline constexpr std::uint32_t kRegWhoami = 0x0000;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCxgbInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cxgb_generic() noexcept {
    return Descriptor{
        .name = "cxgb",
        .provenance = "hbsd/src/sys/dev/cxgb/cxgb_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1425,
        .device_id = 0x0001,
        .init_sequence = kCxgbInit,
        .reset_sequence = kCxgbInit,
    };
}

} // namespace pbsd::uda::cxgb
