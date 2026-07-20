module;
#include <cstdint>

export module pbsd.uda.cfumass;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/usb/storage/cfumass.c
export namespace pbsd::uda::cfumass {

inline constexpr std::uint8_t kCfumassEnable = 0x01;

[[nodiscard]] inline Status validate_lun(unsigned lun) noexcept {
    return lun < 16 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kCfumassInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor cfumass_generic() noexcept {
    return Descriptor{
        .name = "cfumass",
        .provenance = "hbsd/src/sys/dev/usb/storage/cfumass.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kCfumassInit,
        .reset_sequence = kCfumassInit,
    };
}

} // namespace pbsd::uda::cfumass
