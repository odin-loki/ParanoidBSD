module;
#include <cstdint>

export module pbsd.uda.virtio_scsi;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/scsi/virtio_scsi.h
export namespace pbsd::uda::virtio::scsi {

inline constexpr std::uint32_t kFeatureInout   = 0x0001;
inline constexpr std::uint32_t kFeatureHotplug = 0x0002;
inline constexpr std::uint32_t kFeatureChange  = 0x0004;
inline constexpr std::uint32_t kFeatureT10Pi   = 0x0008;

inline constexpr std::uint32_t kCdbSize   = 32;
inline constexpr std::uint32_t kSenseSize = 96;

inline constexpr std::uint8_t kRespOk = 0;

inline constexpr RegInsn kVirtioScsiInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioScsiReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_scsi_1_0() noexcept {
    return Descriptor{
        .name = "virtio-scsi-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/scsi/virtio_scsi.h",
        .device_class = DeviceClass::Block,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevScsi,
        .init_sequence = kVirtioScsiInit,
        .reset_sequence = kVirtioScsiReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevScsi;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdScsi);
}

} // namespace pbsd::uda::virtio::scsi
