module;
#include <cstdint>

export module pbsd.uda.my;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/my/if_my.c
export namespace pbsd::uda::my {

inline constexpr std::uint32_t kRegControl = 0x0000;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 32 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMyInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor my_generic() noexcept {
    return Descriptor{
        .name = "my",
        .provenance = "hbsd/src/sys/dev/my/if_my.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1516,
        .device_id = 0x0800,
        .init_sequence = kMyInit,
        .reset_sequence = kMyInit,
    };
}

} // namespace pbsd::uda::my
