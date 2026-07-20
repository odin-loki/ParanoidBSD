module;
#include <cstdint>

export module pbsd.uda.rl;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/rl/if_rl.c
export namespace pbsd::uda::rl {

inline constexpr std::uint32_t kRegCmd = 0x0037;

[[nodiscard]] inline Status validate_frame(unsigned len) noexcept {
    return len >= 60 && len <= 1518 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kRlInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor rl_generic() noexcept {
    return Descriptor{
        .name = "rl",
        .provenance = "hbsd/src/sys/dev/rl/if_rl.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x10ec,
        .device_id = 0x8139,
        .init_sequence = kRlInit,
        .reset_sequence = kRlInit,
    };
}

} // namespace pbsd::uda::rl
