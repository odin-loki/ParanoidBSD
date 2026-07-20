module;
#include <cstdint>

export module pbsd.uda.dpms;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/dpms/dpms.c
export namespace pbsd::uda::dpms {

enum class Mode : unsigned {
    On          = 0,
    Standby     = 1,
    Suspend     = 2,
    Off         = 3,
};

struct SoftState {
    Mode  mode{Mode::On};
    bool  attached{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.mode = Mode::On;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status set_mode(SoftState& sc, Mode mode) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.mode = mode;
    return Status::Ok;
}

inline constexpr RegInsn kDpmsInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor dpms_generic() noexcept {
    return Descriptor{
        .name = "dpms",
        .provenance = "hbsd/src/sys/dev/dpms/dpms.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kDpmsInit,
        .reset_sequence = kDpmsInit,
    };
}

} // namespace pbsd::uda::dpms
