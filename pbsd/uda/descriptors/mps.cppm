module;
#include <cstdint>

export module pbsd.uda.mps;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mps/mps.c
export namespace pbsd::uda::mps {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_req(unsigned req) noexcept {
    return req < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMpsInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mps_generic() noexcept {
    return Descriptor{
        .name = "mps",
        .provenance = "hbsd/src/sys/dev/mps/mps.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x005c,
        .init_sequence = kMpsInit,
        .reset_sequence = kMpsInit,
    };
}

} // namespace pbsd::uda::mps
