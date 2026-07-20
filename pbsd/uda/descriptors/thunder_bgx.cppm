module;
#include <cstdint>

export module pbsd.uda.thunder_bgx;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vnic/thunder_bgx.c
export namespace pbsd::uda::thunder_bgx {

inline constexpr std::uint32_t kRegBgxCfg = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_lmac(unsigned lmac) noexcept {
    return lmac < 4 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kThunderBgxInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor thunder_bgx_generic() noexcept {
    return Descriptor{
        .name = "thunder_bgx",
        .provenance = "hbsd/src/sys/dev/vnic/thunder_bgx.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x177d,
        .device_id = 0xA026,
        .init_sequence = kThunderBgxInit,
        .reset_sequence = kThunderBgxInit,
    };
}

} // namespace pbsd::uda::thunder_bgx
