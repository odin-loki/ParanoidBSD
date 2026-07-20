module;
#include <cstdint>

export module pbsd.uda.blkfront;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/xen/blkfront/blkfront.c
export namespace pbsd::uda::blkfront {

inline constexpr std::uint32_t kRegSector = 0x0000;
inline constexpr std::uint32_t kCtrlReset = 0x00000001u;

[[nodiscard]] inline Status validate_sector(unsigned count) noexcept {
    return count > 0 && count <= 256 ? Status::Ok : Status::Invalid;
}

inline constexpr RegInsn kBlkfrontInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor blkfront_generic() noexcept {
    return Descriptor{
        .name = "blkfront",
        .provenance = "hbsd/src/sys/dev/xen/blkfront/blkfront.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kBlkfrontInit,
        .reset_sequence = kBlkfrontInit,
    };
}

} // namespace pbsd::uda::blkfront
