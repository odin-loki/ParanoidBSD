module;
#include <cstdint>

export module pbsd.uda.hpt27xx;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hpt27xx/hpt27xx.c
export namespace pbsd::uda::hpt27xx {

inline constexpr std::uint32_t kRegControl = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kHpt27xxInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hpt27xx_generic() noexcept {
    return Descriptor{
        .name = "hpt27xx",
        .provenance = "hbsd/src/sys/dev/hpt27xx/hpt27xx.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1103,
        .device_id = 0x2720,
        .init_sequence = kHpt27xxInit,
        .reset_sequence = kHpt27xxInit,
    };
}

} // namespace pbsd::uda::hpt27xx
