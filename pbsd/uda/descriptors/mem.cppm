module;
#include <cstdint>

export module pbsd.uda.mem;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/mem/mem.c
export namespace pbsd::uda::mem {

struct SoftState {
    std::uint64_t mapped_bytes{};
    bool          attached{};
};

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.mapped_bytes = 0;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status map(SoftState& sc, unsigned len) noexcept {
    if (!sc.attached || len == 0) {
        return Status::Invalid;
    }
    sc.mapped_bytes += len;
    return Status::Ok;
}

inline constexpr RegInsn kMemInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mem_dev() noexcept {
    return Descriptor{
        .name = "mem",
        .provenance = "hbsd/src/sys/dev/mem/mem.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kMemInit,
        .reset_sequence = kMemInit,
    };
}

} // namespace pbsd::uda::mem
