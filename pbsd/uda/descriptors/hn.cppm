module;
#include <cstdint>

export module pbsd.uda.hn;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hyperv/netvsc/if_hn.c
export namespace pbsd::uda::hn {

inline constexpr std::uint32_t kRegVfId = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 64 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kHnInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hn_generic() noexcept {
    return Descriptor{
        .name = "hn",
        .provenance = "hbsd/src/sys/dev/hyperv/netvsc/if_hn.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1414,
        .device_id = 0x0001,
        .init_sequence = kHnInit,
        .reset_sequence = kHnInit,
    };
}

} // namespace pbsd::uda::hn
