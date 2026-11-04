module;
#include <cstdint>

export module pbsd.uda.netfront;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/xen/netfront/netfront.c
export namespace pbsd::uda::netfront {

inline constexpr std::uint32_t kRegTxRing = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_queue(unsigned queue) noexcept {
    return queue < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kNetfrontInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor netfront_generic() noexcept {
    return Descriptor{
        .name = "netfront",
        .provenance = "hbsd/src/sys/dev/xen/netfront/netfront.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kNetfrontInit,
        .reset_sequence = kNetfrontInit,
    };
}

} // namespace pbsd::uda::netfront
