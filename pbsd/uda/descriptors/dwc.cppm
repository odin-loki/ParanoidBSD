module;
#include <cstdint>

export module pbsd.uda.dwc;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/dwc/if_dwc.c
export namespace pbsd::uda::dwc {

inline constexpr std::uint32_t kRegMacCfg = 0x0000;

[[nodiscard]] inline Status validate_mii(unsigned reg) noexcept {
    return reg <= 31 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kDwcInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor dwc_generic() noexcept {
    return Descriptor{
        .name = "dwc",
        .provenance = "hbsd/src/sys/dev/dwc/if_dwc.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kDwcInit,
        .reset_sequence = kDwcInit,
    };
}

} // namespace pbsd::uda::dwc
