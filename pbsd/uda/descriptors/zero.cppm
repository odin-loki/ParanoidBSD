module;
#include <cstdint>

export module pbsd.uda.zero;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/zero/zero.c
export namespace pbsd::uda::zero {

struct SoftState {
    std::uint64_t bytes_read{};
    bool          attached{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.bytes_read = 0;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status read(SoftState& sc, unsigned len) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.bytes_read += len;
    return Status::Ok;
}

inline constexpr RegInsn kZeroInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor zero_dev() noexcept {
    return Descriptor{
        .name = "zero",
        .provenance = "hbsd/src/sys/dev/zero/zero.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kZeroInit,
        .reset_sequence = kZeroInit,
    };
}

} // namespace pbsd::uda::zero
