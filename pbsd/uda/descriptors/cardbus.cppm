module;
#include <cstdint>

export module pbsd.uda.cardbus;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/cardbus/cardbus.c
export namespace pbsd::uda::cardbus {

inline constexpr unsigned kCbMaxSlots = 8;

[[nodiscard]] inline Status validate_slot(unsigned slot) noexcept {
    return slot < kCbMaxSlots ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCardbusInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cardbus_generic() noexcept {
    return Descriptor{
        .name = "cardbus",
        .provenance = "hbsd/src/sys/dev/cardbus/cardbus.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kCardbusInit,
        .reset_sequence = kCardbusInit,
    };
}

} // namespace pbsd::uda::cardbus
