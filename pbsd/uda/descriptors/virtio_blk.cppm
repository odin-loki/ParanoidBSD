module;
#include <cstdint>

export module pbsd.uda.virtio_blk;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

export namespace pbsd::uda::virtio::blk {

// PROVENANCE: hbsd/src/sys/dev/virtio/block/virtio_blk.h
inline constexpr std::uint32_t kFeatureSizeMax     = 0x0002;
inline constexpr std::uint32_t kFeatureSegMax      = 0x0004;
inline constexpr std::uint32_t kFeatureGeometry    = 0x0010;
inline constexpr std::uint32_t kFeatureRo          = 0x0020;
inline constexpr std::uint32_t kFeatureBlkSize     = 0x0040;
inline constexpr std::uint32_t kFeatureFlush       = 0x0200;
inline constexpr std::uint32_t kFeatureTopology    = 0x0400;
inline constexpr std::uint32_t kFeatureConfigWce   = 0x0800;
inline constexpr std::uint32_t kFeatureMq          = 0x1000;
inline constexpr std::uint32_t kFeatureDiscard     = 0x2000;
inline constexpr std::uint32_t kFeatureWriteZeroes = 0x4000;

inline constexpr std::uint32_t kCmdIn           = 0;
inline constexpr std::uint32_t kCmdOut          = 1;
inline constexpr std::uint32_t kCmdFlush        = 4;
inline constexpr std::uint32_t kCmdGetId        = 8;
inline constexpr std::uint32_t kCmdDiscard      = 11;
inline constexpr std::uint32_t kCmdWriteZeroes  = 13;

inline constexpr std::uint8_t kStatusOk      = 0;
inline constexpr std::uint8_t kStatusIoerr   = 1;
inline constexpr std::uint8_t kStatusUnsupp  = 2;

inline constexpr std::size_t kIdBytes = 20;

inline constexpr RegInsn kVirtioBlkInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioBlkReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_blk_1_0() noexcept {
    return Descriptor{
        .name = "virtio-blk-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/block/virtio_blk.h",
        .device_class = DeviceClass::Block,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevBlk,
        .init_sequence = kVirtioBlkInit,
        .reset_sequence = kVirtioBlkReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevBlk;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdBlock);
}

} // namespace pbsd::uda::virtio::blk
