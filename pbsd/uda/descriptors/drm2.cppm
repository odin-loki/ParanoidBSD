module;
#include <cstdint>

export module pbsd.uda.drm2;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/drm2/drm2.c
export namespace pbsd::uda::drm2 {

inline constexpr unsigned kDrmMaxCrtcs = 4;

[[nodiscard]] inline Status validate_crtc(unsigned crtc) noexcept {
    return crtc < kDrmMaxCrtcs ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kDrm2Init[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor drm2_generic() noexcept {
    return Descriptor{
        .name = "drm2",
        .provenance = "hbsd/src/sys/dev/drm2/drm2.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kDrm2Init,
        .reset_sequence = kDrm2Init,
    };
}

} // namespace pbsd::uda::drm2
