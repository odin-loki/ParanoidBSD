module;
#include <cstdint>

export module pbsd.uda.simplefb;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vt/hw/simplefb/simplefb.c
export namespace pbsd::uda::simplefb {

inline constexpr std::uint32_t kRegFbBase = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_mode(unsigned width, unsigned height) noexcept {
    return width >= 640 && height >= 480 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kSimplefbInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor simplefb_generic() noexcept {
    return Descriptor{
        .name = "simplefb",
        .provenance = "hbsd/src/sys/dev/vt/hw/simplefb/simplefb.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kSimplefbInit,
        .reset_sequence = kSimplefbInit,
    };
}

} // namespace pbsd::uda::simplefb
