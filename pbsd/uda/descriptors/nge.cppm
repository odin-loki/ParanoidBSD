module;
#include <cstdint>

export module pbsd.uda.nge;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/nge/if_nge.c
export namespace pbsd::uda::nge {

inline constexpr std::uint32_t kRegTxPoll = 0x0038;

[[nodiscard]] inline Status validate_unit(int unit) noexcept {
    return unit >= 0 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kNgeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nge_generic() noexcept {
    return Descriptor{
        .name = "nge",
        .provenance = "hbsd/src/sys/dev/nge/if_nge.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10ec,
        .device_id = 0x8169,
        .init_sequence = kNgeInit,
        .reset_sequence = kNgeInit,
    };
}

} // namespace pbsd::uda::nge
