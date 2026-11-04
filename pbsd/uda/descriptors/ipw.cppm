module;
#include <cstdint>

export module pbsd.uda.ipw;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ipw/if_ipw.c
export namespace pbsd::uda::ipw {

inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_channel(unsigned ch) noexcept {
    return ch < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIpwInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ipw_generic() noexcept {
    return Descriptor{
        .name = "ipw",
        .provenance = "hbsd/src/sys/dev/ipw/if_ipw.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x4220,
        .init_sequence = kIpwInit,
        .reset_sequence = kIpwInit,
    };
}

} // namespace pbsd::uda::ipw
