module;
#include <cstdint>

export module pbsd.uda.firewire;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/firewire/firewire.c
export namespace pbsd::uda::firewire {

inline constexpr unsigned kFwMaxNodes = 63;

[[nodiscard]] inline Status validate_node(unsigned node) noexcept {
    return node < kFwMaxNodes ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kFirewireInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor firewire_generic() noexcept {
    return Descriptor{
        .name = "firewire",
        .provenance = "hbsd/src/sys/dev/firewire/firewire.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kFirewireInit,
        .reset_sequence = kFirewireInit,
    };
}

} // namespace pbsd::uda::firewire
