module;
#include <cstdint>

export module pbsd.uda.efifb;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vt/hw/efifb/efifb.c
export namespace pbsd::uda::efifb {

inline constexpr std::uint32_t kRegMode = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_mode(unsigned width, unsigned height) noexcept {
    return width >= 640 && height >= 480 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kEfifbInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor efifb_generic() noexcept {
    return Descriptor{
        .name = "efifb",
        .provenance = "hbsd/src/sys/dev/vt/hw/efifb/efifb.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kEfifbInit,
        .reset_sequence = kEfifbInit,
    };
}

} // namespace pbsd::uda::efifb
