module;
#include <cstdint>

export module pbsd.uda.null;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/null/null.c
export namespace pbsd::uda::null {

struct SoftState {
    std::uint64_t bytes_written{};
    std::uint64_t bytes_read{};
    bool          attached{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.bytes_written = 0;
    sc.bytes_read = 0;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status write(SoftState& sc, unsigned len) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.bytes_written += len;
    return Status::Ok;
}

[[nodiscard]] inline Status read(SoftState& sc, unsigned len) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.bytes_read += len;
    return Status::Ok;
}

inline constexpr RegInsn kNullInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor null_dev() noexcept {
    return Descriptor{
        .name = "null",
        .provenance = "hbsd/src/sys/dev/null/null.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kNullInit,
        .reset_sequence = kNullInit,
    };
}

} // namespace pbsd::uda::null
