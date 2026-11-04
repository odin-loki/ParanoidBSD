module;
#include <cstdint>

export module pbsd.uda.sge;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sge/if_sge.c
export namespace pbsd::uda::sge {

inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_port(unsigned port) noexcept {
    return port < 2 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kSgeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor sge_generic() noexcept {
    return Descriptor{
        .name = "sge",
        .provenance = "hbsd/src/sys/dev/sge/if_sge.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10ec,
        .device_id = 0x8168,
        .init_sequence = kSgeInit,
        .reset_sequence = kSgeInit,
    };
}

} // namespace pbsd::uda::sge
