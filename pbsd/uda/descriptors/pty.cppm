module;
#include <cstdint>

export module pbsd.uda.pty;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/pty/pty.c — legacy BSD PTY compatibility layer.
export namespace pbsd::uda::pty {

inline constexpr unsigned kNameLen = 6;
inline constexpr unsigned kDefaultWarningCnt = 1;

struct SoftState {
    bool     in_use{};
    unsigned warning_cnt{kDefaultWarningCnt};
};

[[nodiscard]] inline Status alloc(SoftState& sc) noexcept {
    if (sc.in_use) {
        return Status::Busy;
    }
    sc.in_use = true;
    return Status::Ok;
}

[[nodiscard]] inline Status release(SoftState& sc) noexcept {
    if (!sc.in_use) {
        return Status::Invalid;
    }
    sc.in_use = false;
    return Status::Ok;
}

inline constexpr RegInsn kPtyInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor pty_legacy() noexcept {
    return Descriptor{
        .name = "pty",
        .provenance = "hbsd/src/sys/dev/pty/pty.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kPtyInit,
        .reset_sequence = kPtyInit,
    };
}

[[nodiscard]] inline constexpr Descriptor ptmx_node() noexcept {
    return Descriptor{
        .name = "ptmx",
        .provenance = "hbsd/src/sys/dev/pty/pty.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kPtyInit,
        .reset_sequence = kPtyInit,
    };
}

} // namespace pbsd::uda::pty
