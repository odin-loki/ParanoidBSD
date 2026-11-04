module;
#include <cstdint>

export module pbsd.uda.bxe;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bxe/bxe.c
export namespace pbsd::uda::bxe {

inline constexpr std::uint32_t kRegMcp = 0x0000;

[[nodiscard]] inline Status validate_function(unsigned fn) noexcept {
    return fn < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kBxeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bxe_generic() noexcept {
    return Descriptor{
        .name = "bxe",
        .provenance = "hbsd/src/sys/dev/bxe/bxe.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x14e4,
        .device_id = 0x164f,
        .init_sequence = kBxeInit,
        .reset_sequence = kBxeInit,
    };
}

} // namespace pbsd::uda::bxe
