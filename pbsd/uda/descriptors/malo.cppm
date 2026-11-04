module;
#include <cstdint>

export module pbsd.uda.malo;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/malo/if_malo.c
export namespace pbsd::uda::malo {

inline constexpr std::uint32_t kRegIntr = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_vap(unsigned vap) noexcept {
    return vap < 8 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kMaloInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor malo_generic() noexcept {
    return Descriptor{
        .name = "malo",
        .provenance = "hbsd/src/sys/dev/malo/if_malo.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x11ab,
        .device_id = 0x2a0b,
        .init_sequence = kMaloInit,
        .reset_sequence = kMaloInit,
    };
}

} // namespace pbsd::uda::malo
