module;
#include <cstdint>

export module pbsd.uda.le;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/le/if_le_pci.c
export namespace pbsd::uda::le {

inline constexpr std::uint32_t kRegRdp = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_ring(unsigned ring) noexcept {
    return ring < 64 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kLeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor le_generic() noexcept {
    return Descriptor{
        .name = "le",
        .provenance = "hbsd/src/sys/dev/le/if_le_pci.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1022,
        .device_id = 0x2000,
        .init_sequence = kLeInit,
        .reset_sequence = kLeInit,
    };
}

} // namespace pbsd::uda::le
