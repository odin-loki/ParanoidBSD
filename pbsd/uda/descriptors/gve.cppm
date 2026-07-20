module;
#include <cstdint>

export module pbsd.uda.gve;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/gve/gve_main.c
export namespace pbsd::uda::gve {

inline constexpr std::uint32_t kRegDmaMode = 0x0010;

[[nodiscard]] inline Status validate_queue(unsigned q) noexcept {
    return q < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kGveInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor gve_generic() noexcept {
    return Descriptor{
        .name = "gve",
        .provenance = "hbsd/src/sys/dev/gve/gve_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1ae0,
        .device_id = 0x0042,
        .init_sequence = kGveInit,
        .reset_sequence = kGveInit,
    };
}

} // namespace pbsd::uda::gve
