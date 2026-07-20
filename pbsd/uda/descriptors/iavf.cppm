module;
#include <cstdint>

export module pbsd.uda.iavf;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/iavf/if_iavf_iflib.c
export namespace pbsd::uda::iavf {

inline constexpr std::uint32_t kRegIntMask = 0x0380;

[[nodiscard]] inline Status validate_vector(unsigned vec) noexcept {
    return vec < 64 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIavfInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iavf_generic() noexcept {
    return Descriptor{
        .name = "iavf",
        .provenance = "hbsd/src/sys/dev/iavf/if_iavf_iflib.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x154c,
        .init_sequence = kIavfInit,
        .reset_sequence = kIavfInit,
    };
}

} // namespace pbsd::uda::iavf
