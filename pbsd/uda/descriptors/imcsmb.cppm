module;
#include <cstdint>

export module pbsd.uda.imcsmb;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/imcsmb/imcsmb.c
export namespace pbsd::uda::imcsmb {

inline constexpr unsigned kSmbMaxBytes = 32;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kSmbMaxBytes ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kImcsmbInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor imcsmb_generic() noexcept {
    return Descriptor{
        .name = "imcsmb",
        .provenance = "hbsd/src/sys/dev/imcsmb/imcsmb.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x8086,
        .device_id = 0x0c02,
        .init_sequence = kImcsmbInit,
        .reset_sequence = kImcsmbInit,
    };
}

} // namespace pbsd::uda::imcsmb
