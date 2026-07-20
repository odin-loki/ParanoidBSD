module;
#include <cstdint>

export module pbsd.uda.aac;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/aac/aac.c
export namespace pbsd::uda::aac {

inline constexpr std::uint32_t kRegDoorbell = 0x0020;

[[nodiscard]] inline Status validate_fib(unsigned idx) noexcept {
    return idx < 512 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kAacInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor aac_generic() noexcept {
    return Descriptor{
        .name = "aac",
        .provenance = "hbsd/src/sys/dev/aac/aac.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x9005,
        .device_id = 0x0285,
        .init_sequence = kAacInit,
        .reset_sequence = kAacInit,
    };
}

} // namespace pbsd::uda::aac
