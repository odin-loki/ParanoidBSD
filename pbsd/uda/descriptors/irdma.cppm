module;
#include <cstdint>

export module pbsd.uda.irdma;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/irdma/irdma_main.c
export namespace pbsd::uda::irdma {

inline constexpr std::uint32_t kRegHwVer = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_qp(unsigned qp) noexcept {
    return qp < 4096 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kIrdmaInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor irdma_generic() noexcept {
    return Descriptor{
        .name = "irdma",
        .provenance = "hbsd/src/sys/dev/irdma/irdma_main.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x8086,
        .device_id = 0x1593,
        .init_sequence = kIrdmaInit,
        .reset_sequence = kIrdmaInit,
    };
}

} // namespace pbsd::uda::irdma
