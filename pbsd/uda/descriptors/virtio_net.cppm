module;
#include <cstdint>

export module pbsd.uda.virtio_net;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

export namespace pbsd::uda::virtio::net {

// PROVENANCE: hbsd/src/sys/dev/virtio/network/virtio_net.h
inline constexpr std::uint64_t kFeatureCsum              = 1ULL << 0;
inline constexpr std::uint64_t kFeatureGuestCsum         = 1ULL << 1;
inline constexpr std::uint64_t kFeatureCtrlGuestOffloads = 1ULL << 2;
inline constexpr std::uint64_t kFeatureMtu               = 1ULL << 3;
inline constexpr std::uint64_t kFeatureMac               = 1ULL << 5;
inline constexpr std::uint64_t kFeatureGso               = 1ULL << 6;
inline constexpr std::uint64_t kFeatureGuestTso4         = 1ULL << 7;
inline constexpr std::uint64_t kFeatureGuestTso6         = 1ULL << 8;
inline constexpr std::uint64_t kFeatureMrgRxbuf          = 1ULL << 15;
inline constexpr std::uint64_t kFeatureStatus            = 1ULL << 16;
inline constexpr std::uint64_t kFeatureCtrlVq            = 1ULL << 17;
inline constexpr std::uint64_t kFeatureMq                = 1ULL << 22;
inline constexpr std::uint64_t kFeatureSpeedDuplex       = 1ULL << 63;

inline constexpr std::uint16_t kLinkUp     = 1;
inline constexpr std::uint16_t kAnnounce   = 2;

inline constexpr std::uint8_t kHdrNeedsCsum  = 1;
inline constexpr std::uint8_t kHdrDataValid  = 2;
inline constexpr std::uint8_t kGsoNone        = 0;
inline constexpr std::uint8_t kGsoTcpv4      = 1;
inline constexpr std::uint8_t kGsoUdp        = 3;
inline constexpr std::uint8_t kGsoTcpv6      = 4;

inline constexpr RegInsn kVirtioNetInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioNetReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_net_1_0() noexcept {
    return Descriptor{
        .name = "virtio-net-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/network/virtio_net.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevNet,
        .init_sequence = kVirtioNetInit,
        .reset_sequence = kVirtioNetReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevNet;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdNetwork);
}

} // namespace pbsd::uda::virtio::net
