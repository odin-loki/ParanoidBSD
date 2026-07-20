module;
#include <cstdint>

export module pbsd.uda.videomode;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/videomode/videomode.c
export namespace pbsd::uda::videomode {

inline constexpr unsigned kFlagPhsync   = 1u << 0;
inline constexpr unsigned kFlagNhsync   = 1u << 1;
inline constexpr unsigned kFlagPvsync   = 1u << 2;
inline constexpr unsigned kFlagNvsync   = 1u << 3;
inline constexpr unsigned kFlagInterlace = 1u << 4;
inline constexpr unsigned kFlagDblscan  = 1u << 5;

struct Mode {
    unsigned dot_clock{};
    unsigned hdisplay{};
    unsigned hsync_start{};
    unsigned hsync_end{};
    unsigned htotal{};
    unsigned vdisplay{};
    unsigned vsync_start{};
    unsigned vsync_end{};
    unsigned vtotal{};
    unsigned flags{};
    const char* name{nullptr};
};

inline constexpr Mode k640x480x60{
    .dot_clock = 25175,
    .hdisplay = 640,
    .hsync_start = 656,
    .hsync_end = 752,
    .htotal = 800,
    .vdisplay = 480,
    .vsync_start = 490,
    .vsync_end = 492,
    .vtotal = 525,
    .flags = kFlagNhsync | kFlagNvsync,
    .name = "640x480x60",
};

[[nodiscard]] inline Status validate_mode(const Mode& m) noexcept {
    if (m.hdisplay == 0 || m.vdisplay == 0 || m.htotal == 0 || m.vtotal == 0) {
        return Status::Invalid;
    }
    if (m.hsync_end <= m.hsync_start || m.vsync_end <= m.vsync_start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

inline constexpr RegInsn kVideomodeInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor videomode_generic() noexcept {
    return Descriptor{
        .name = "videomode",
        .provenance = "hbsd/src/sys/dev/videomode/videomode.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kVideomodeInit,
        .reset_sequence = kVideomodeInit,
    };
}

} // namespace pbsd::uda::videomode
