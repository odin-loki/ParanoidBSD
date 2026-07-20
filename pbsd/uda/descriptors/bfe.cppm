module;
#include <cstdint>

export module pbsd.uda.bfe;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bfe/if_bfe.c
export namespace pbsd::uda::bfe {

inline constexpr std::uint32_t kRegMode = 0x006c;

[[nodiscard]] inline Status validate_frame(unsigned len) noexcept {
    return len >= 60 && len <= 1518 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kBfeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bfe_generic() noexcept {
    return Descriptor{
        .name = "bfe",
        .provenance = "hbsd/src/sys/dev/bfe/if_bfe.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x14e4,
        .device_id = 0x4401,
        .init_sequence = kBfeInit,
        .reset_sequence = kBfeInit,
    };
}

} // namespace pbsd::uda::bfe
