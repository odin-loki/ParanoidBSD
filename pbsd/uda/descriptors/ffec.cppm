module;
#include <cstdint>

export module pbsd.uda.ffec;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ffec/if_ffec.c
export namespace pbsd::uda::ffec {

inline constexpr std::uint32_t kRegIcr = 0x00b0;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kFfecInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ffec_generic() noexcept {
    return Descriptor{
        .name = "ffec",
        .provenance = "hbsd/src/sys/dev/ffec/if_ffec.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1957,
        .device_id = 0x7890,
        .init_sequence = kFfecInit,
        .reset_sequence = kFfecInit,
    };
}

} // namespace pbsd::uda::ffec
