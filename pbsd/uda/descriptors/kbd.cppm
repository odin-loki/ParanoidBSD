module;
#include <cstdint>

export module pbsd.uda.kbd;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/kbd/kbd.c
export namespace pbsd::uda::kbd {

inline constexpr std::uint32_t kRegData = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_scancode(unsigned scancode) noexcept {
    return scancode < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kKbdInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor kbd_generic() noexcept {
    return Descriptor{
        .name = "kbd",
        .provenance = "hbsd/src/sys/dev/kbd/kbd.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kKbdInit,
        .reset_sequence = kKbdInit,
    };
}

} // namespace pbsd::uda::kbd
