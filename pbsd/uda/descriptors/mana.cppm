module;
#include <cstdint>

export module pbsd.uda.mana;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mana/gdma_main.c
export namespace pbsd::uda::mana {

inline constexpr std::uint32_t kRegGdma = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_queue(unsigned queue) noexcept {
    return queue < 64 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kManaInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mana_generic() noexcept {
    return Descriptor{
        .name = "mana",
        .provenance = "hbsd/src/sys/dev/mana/gdma_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x1414,
        .device_id = 0x0100,
        .init_sequence = kManaInit,
        .reset_sequence = kManaInit,
    };
}

} // namespace pbsd::uda::mana
