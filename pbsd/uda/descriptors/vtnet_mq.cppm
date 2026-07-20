module;
#include <cstdint>

export module pbsd.uda.vtnet_mq;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/network/{if_vtnetvar.h,if_vtnet.c,virtio_net.h}
export namespace pbsd::uda::vtnet_mq {

using virtio::kMmioStatus;
using virtio::kMmioGuestFeatures;
using virtio::kMmioQueueSel;
using virtio::kMmioQueueReady;
using virtio::kStatusAck;
using virtio::kStatusDriver;
using virtio::kFeaturesOk;
using virtio::kStatusDriverOk;
using virtio::kStatusReset;
using virtio::kPciVendorRedHat;
using virtio::kPciDevNet;
using virtio::kIdNetwork;
using virtio::probe_mmio_device;

inline constexpr std::uint64_t kFeatureMq          = 1ULL << 22;
inline constexpr std::uint64_t kFeatureSpeedDuplex = 1ULL << 63;
inline constexpr std::uint64_t kFeatureCtrlMacAddr = 1ULL << 23;
inline constexpr std::uint64_t kFeatureCtrlVlan    = 1ULL << 24;
inline constexpr std::uint64_t kFeatureGuestAnnounce = 1ULL << 21;
inline constexpr std::uint64_t kRingEventIdx       = 1ULL << 29;
inline constexpr std::uint64_t kRingIndirectDesc   = 1ULL << 28;

inline constexpr std::uint64_t kModernFeatures =
    kFeatureMq | kFeatureSpeedDuplex | kFeatureCtrlMacAddr |
    kFeatureCtrlVlan | kFeatureGuestAnnounce |
    kRingEventIdx | kRingIndirectDesc;

inline constexpr std::uint32_t kMmioQueueNumMax = 0x034;
inline constexpr std::uint16_t kDefaultMqPairs = 4;

inline constexpr std::uint16_t kMinMtu = 68;
inline constexpr std::uint32_t kMaxMtu = 65536;
inline constexpr std::uint32_t kMaxRxSize = 65550;

inline constexpr RegInsn kVtnetMqInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioGuestFeatures,
     static_cast<std::uint32_t>(kModernFeatures & 0xFFFFFFFFu), 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioQueueSel, 0, 0, 0},
    {RegOp::Write32, kMmioQueueReady, 0x1, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVtnetMqReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vtnet_mq_modern() noexcept {
    return Descriptor{
        .name = "vtnet-mq-modern",
        .provenance = "hbsd/src/sys/dev/virtio/network/if_vtnetvar.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevNet,
        .init_sequence = kVtnetMqInit,
        .reset_sequence = kVtnetMqReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevNet;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return probe_mmio_device(mem, kIdNetwork);
}

} // namespace pbsd::uda::vtnet_mq
