module;
#include <cstdint>

export module pbsd.uda.virtio_fs;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/fs/virtio_fs.c, virtio_ids.h
export namespace pbsd::uda::virtio::fs {

inline constexpr std::uint32_t kIdFs = 26;
inline constexpr std::uint16_t kPciDevFs = 0x101B;

inline constexpr RegInsn kVirtioFsInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioFsReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_fs_1_0() noexcept {
    return Descriptor{
        .name = "virtio-fs-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/fs/virtio_fs.c",
        .device_class = DeviceClass::Block,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevFs,
        .init_sequence = kVirtioFsInit,
        .reset_sequence = kVirtioFsReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdFs);
}

} // namespace pbsd::uda::virtio::fs
