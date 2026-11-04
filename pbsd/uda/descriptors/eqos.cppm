module;
#include <cstdint>

export module pbsd.uda.eqos;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/eqos/if_eqos.c
export namespace pbsd::uda::eqos {

inline constexpr std::uint32_t kRegDmaMode = 0x1000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kEqosInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor eqos_generic() noexcept {
    return Descriptor{
        .name = "eqos",
        .provenance = "hbsd/src/sys/dev/eqos/if_eqos.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x7026,
        .device_id = 0x0100,
        .init_sequence = kEqosInit,
        .reset_sequence = kEqosInit,
    };
}

} // namespace pbsd::uda::eqos
