module;
#include <cstdint>

export module pbsd.uda.iwx;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/iwx/if_iwx.c
export namespace pbsd::uda::iwx {

inline constexpr std::uint32_t kRegCmd = 0x0000;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIwxInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iwx_generic() noexcept {
    return Descriptor{
        .name = "iwx",
        .provenance = "hbsd/src/sys/dev/iwx/if_iwx.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x2526,
        .init_sequence = kIwxInit,
        .reset_sequence = kIwxInit,
    };
}

} // namespace pbsd::uda::iwx
