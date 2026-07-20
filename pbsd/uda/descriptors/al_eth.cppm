module;
#include <cstdint>

export module pbsd.uda.al_eth;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/al_eth/al_eth.c
export namespace pbsd::uda::al_eth {

inline constexpr std::uint32_t kRegAdapterCtl = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kAlEthInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor al_eth_generic() noexcept {
    return Descriptor{
        .name = "al_eth",
        .provenance = "hbsd/src/sys/dev/al_eth/al_eth.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1c36,
        .device_id = 0x0003,
        .init_sequence = kAlEthInit,
        .reset_sequence = kAlEthInit,
    };
}

} // namespace pbsd::uda::al_eth
