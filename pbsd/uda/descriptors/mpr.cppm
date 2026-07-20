module;
#include <cstdint>

export module pbsd.uda.mpr;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mpr/mpr.c
export namespace pbsd::uda::mpr {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_req(unsigned req) noexcept {
    return req < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMprInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mpr_generic() noexcept {
    return Descriptor{
        .name = "mpr",
        .provenance = "hbsd/src/sys/dev/mpr/mpr.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x0056,
        .init_sequence = kMprInit,
        .reset_sequence = kMprInit,
    };
}

} // namespace pbsd::uda::mpr
