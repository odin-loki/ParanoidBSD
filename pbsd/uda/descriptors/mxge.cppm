module;
#include <cstdint>

export module pbsd.uda.mxge;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mxge/if_mxge.c
export namespace pbsd::uda::mxge {

inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_slice(unsigned slice) noexcept {
    return slice < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMxgeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mxge_generic() noexcept {
    return Descriptor{
        .name = "mxge",
        .provenance = "hbsd/src/sys/dev/mxge/if_mxge.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x15ad,
        .device_id = 0x0001,
        .init_sequence = kMxgeInit,
        .reset_sequence = kMxgeInit,
    };
}

} // namespace pbsd::uda::mxge
