module;
#include <cstdint>

export module pbsd.uda.vkbd;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vkbd/vkbd.c
export namespace pbsd::uda::vkbd {

inline constexpr std::uint32_t kRegStatus = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_key(unsigned key) noexcept {
    return key < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kVkbdInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vkbd_generic() noexcept {
    return Descriptor{
        .name = "vkbd",
        .provenance = "hbsd/src/sys/dev/vkbd/vkbd.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kVkbdInit,
        .reset_sequence = kVkbdInit,
    };
}

} // namespace pbsd::uda::vkbd
