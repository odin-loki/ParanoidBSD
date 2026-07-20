module;
#include <cstdint>

export module pbsd.uda.hptiop;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hptiop/hptiop.c
export namespace pbsd::uda::hptiop {

inline constexpr std::uint32_t kRegIopCmd = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_cmd(unsigned cmd) noexcept {
    return cmd < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kHptiopInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hptiop_generic() noexcept {
    return Descriptor{
        .name = "hptiop",
        .provenance = "hbsd/src/sys/dev/hptiop/hptiop.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1103,
        .device_id = 0x4520,
        .init_sequence = kHptiopInit,
        .reset_sequence = kHptiopInit,
    };
}

} // namespace pbsd::uda::hptiop
