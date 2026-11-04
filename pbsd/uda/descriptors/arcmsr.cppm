module;
#include <cstdint>

export module pbsd.uda.arcmsr;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/arcmsr/arcmsr.c
export namespace pbsd::uda::arcmsr {

inline constexpr std::uint32_t kRegOutbound = 0x0040;

[[nodiscard]] inline Status validate_target(unsigned tgt) noexcept {
    return tgt < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kArcmsrInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor arcmsr_generic() noexcept {
    return Descriptor{
        .name = "arcmsr",
        .provenance = "hbsd/src/sys/dev/arcmsr/arcmsr.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x17d3,
        .device_id = 0x1010,
        .init_sequence = kArcmsrInit,
        .reset_sequence = kArcmsrInit,
    };
}

} // namespace pbsd::uda::arcmsr
