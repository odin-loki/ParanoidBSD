module;
#include <cstdint>

export module pbsd.uda.hifn;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hifn/hifn7751.c
export namespace pbsd::uda::hifn {

inline constexpr std::uint32_t kRegCommand = 0x0004;
inline constexpr std::uint32_t kCmdReset = 0x00000001u;

[[nodiscard]] inline Status validate_session(unsigned sid) noexcept {
    return sid < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kHifnInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hifn_generic() noexcept {
    return Descriptor{
        .name = "hifn",
        .provenance = "hbsd/src/sys/dev/hifn/hifn7751.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x13a3,
        .device_id = 0x0002,
        .init_sequence = kHifnInit,
        .reset_sequence = kHifnInit,
    };
}

} // namespace pbsd::uda::hifn
