module;
#include <cstdint>

export module pbsd.uda.mge;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mge/if_mge.c
export namespace pbsd::uda::mge {

inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMgeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mge_generic() noexcept {
    return Descriptor{
        .name = "mge",
        .provenance = "hbsd/src/sys/dev/mge/if_mge.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x111d,
        .device_id = 0x0001,
        .init_sequence = kMgeInit,
        .reset_sequence = kMgeInit,
    };
}

} // namespace pbsd::uda::mge
