module;
#include <cstdint>

export module pbsd.uda.vte;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vte/if_vte.c
export namespace pbsd::uda::vte {

inline constexpr std::uint32_t kRegMacCtl = 0x0008;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kVteInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vte_generic() noexcept {
    return Descriptor{
        .name = "vte",
        .provenance = "hbsd/src/sys/dev/vte/if_vte.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0002,
        .device_id = 0x0003,
        .init_sequence = kVteInit,
        .reset_sequence = kVteInit,
    };
}

} // namespace pbsd::uda::vte
