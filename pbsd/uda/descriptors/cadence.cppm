module;
#include <cstdint>

export module pbsd.uda.cadence;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cadence/cadence.c
export namespace pbsd::uda::cadence {

inline constexpr std::uint32_t kRegStatus = 0x0004;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCadenceInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cadence_generic() noexcept {
    return Descriptor{
        .name = "cadence",
        .provenance = "hbsd/src/sys/dev/cadence/cadence.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x17cd,
        .device_id = 0xdc08,
        .init_sequence = kCadenceInit,
        .reset_sequence = kCadenceInit,
    };
}

} // namespace pbsd::uda::cadence
