module;
#include <cstdint>

export module pbsd.uda.ena;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ena/ena.c
export namespace pbsd::uda::ena {

inline constexpr std::uint32_t kRegDevCtl = 0x0000;
inline constexpr std::uint32_t kDevCtlReset = 0x1;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kEnaInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ena_generic() noexcept {
    return Descriptor{
        .name = "ena",
        .provenance = "hbsd/src/sys/dev/ena/ena.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1d0f,
        .device_id = 0xec20,
        .init_sequence = kEnaInit,
        .reset_sequence = kEnaInit,
    };
}

} // namespace pbsd::uda::ena
