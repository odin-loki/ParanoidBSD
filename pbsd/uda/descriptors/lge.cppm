module;
#include <cstdint>

export module pbsd.uda.lge;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/lge/if_lge.c
export namespace pbsd::uda::lge {

inline constexpr std::uint32_t kRegHostCmd = 0x0000;

[[nodiscard]] inline Status validate_ring(unsigned idx) noexcept {
    return idx < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kLgeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor lge_generic() noexcept {
    return Descriptor{
        .name = "lge",
        .provenance = "hbsd/src/sys/dev/lge/if_lge.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x11ab,
        .device_id = 0x4320,
        .init_sequence = kLgeInit,
        .reset_sequence = kLgeInit,
    };
}

} // namespace pbsd::uda::lge
