module;
#include <cstdint>

export module pbsd.uda.enetc;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/enetc/if_enetc.c
export namespace pbsd::uda::enetc {

inline constexpr std::uint32_t kRegImr = 0x0004;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kEnetcInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor enetc_generic() noexcept {
    return Descriptor{
        .name = "enetc",
        .provenance = "hbsd/src/sys/dev/enetc/if_enetc.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1957,
        .device_id = 0xe100,
        .init_sequence = kEnetcInit,
        .reset_sequence = kEnetcInit,
    };
}

} // namespace pbsd::uda::enetc
