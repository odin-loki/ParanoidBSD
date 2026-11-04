module;
#include <cstdint>

export module pbsd.uda.ips;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ips/ips.c
export namespace pbsd::uda::ips {

inline constexpr std::uint32_t kRegCmd = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_cmd(unsigned cmd) noexcept {
    return cmd < 64 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIpsInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ips_generic() noexcept {
    return Descriptor{
        .name = "ips",
        .provenance = "hbsd/src/sys/dev/ips/ips.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1014,
        .device_id = 0x002E,
        .init_sequence = kIpsInit,
        .reset_sequence = kIpsInit,
    };
}

} // namespace pbsd::uda::ips
