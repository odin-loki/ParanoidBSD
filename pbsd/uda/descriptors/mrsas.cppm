module;
#include <cstdint>

export module pbsd.uda.mrsas;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mrsas/mrsas.c
export namespace pbsd::uda::mrsas {

inline constexpr std::uint32_t kRegInbound = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_frame(unsigned frame) noexcept {
    return frame < 128 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMrsasInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mrsas_generic() noexcept {
    return Descriptor{
        .name = "mrsas",
        .provenance = "hbsd/src/sys/dev/mrsas/mrsas.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x005d,
        .init_sequence = kMrsasInit,
        .reset_sequence = kMrsasInit,
    };
}

} // namespace pbsd::uda::mrsas
