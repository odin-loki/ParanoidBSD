module;
#include <cstdint>

export module pbsd.uda.ae;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ae/if_ae.c
export namespace pbsd::uda::ae {

inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_desc(unsigned idx) noexcept {
    return idx < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kAeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ae_generic() noexcept {
    return Descriptor{
        .name = "ae",
        .provenance = "hbsd/src/sys/dev/ae/if_ae.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10ec,
        .device_id = 0x8168,
        .init_sequence = kAeInit,
        .reset_sequence = kAeInit,
    };
}

} // namespace pbsd::uda::ae
