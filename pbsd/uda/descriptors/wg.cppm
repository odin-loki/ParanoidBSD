module;
#include <cstdint>

export module pbsd.uda.wg;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/wg/if_wg.c
export namespace pbsd::uda::wg {

inline constexpr unsigned kMaxPeers = 256;

[[nodiscard]] inline Status validate_peer(unsigned idx) noexcept {
    return idx < kMaxPeers ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kWgInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor wg_generic() noexcept {
    return Descriptor{
        .name = "wg",
        .provenance = "hbsd/src/sys/dev/wg/if_wg.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kWgInit,
        .reset_sequence = kWgInit,
    };
}

} // namespace pbsd::uda::wg
