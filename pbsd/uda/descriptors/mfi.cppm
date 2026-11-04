module;
#include <cstdint>

export module pbsd.uda.mfi;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/mfi/mfi_pci.c
export namespace pbsd::uda::mfi {

inline constexpr std::uint32_t kRegDoorbell = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_cmd(unsigned cmd) noexcept {
    return cmd < 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMfiInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mfi_generic() noexcept {
    return Descriptor{
        .name = "mfi",
        .provenance = "hbsd/src/sys/dev/mfi/mfi_pci.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x1000,
        .device_id = 0x005b,
        .init_sequence = kMfiInit,
        .reset_sequence = kMfiInit,
    };
}

} // namespace pbsd::uda::mfi
