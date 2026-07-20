module;
#include <cstdint>

export module pbsd.uda.ixv;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ixgbe/if_ixv.c
export namespace pbsd::uda::ixv {

inline constexpr std::uint32_t kRegCtrl = 0x0000;

[[nodiscard]] inline Status validate_vf(unsigned vf) noexcept {
    return vf < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIxvInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ixv_generic() noexcept {
    return Descriptor{
        .name = "ixv",
        .provenance = "hbsd/src/sys/dev/ixgbe/if_ixv.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x10ed,
        .init_sequence = kIxvInit,
        .reset_sequence = kIxvInit,
    };
}

} // namespace pbsd::uda::ixv
