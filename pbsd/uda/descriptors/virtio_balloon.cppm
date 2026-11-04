module;
#include <cstdint>

export module pbsd.uda.virtio_balloon;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/balloon/virtio_balloon.h
export namespace pbsd::uda::virtio::balloon {

inline constexpr std::uint32_t kFeatureMustTellHost  = 0x1;
inline constexpr std::uint32_t kFeatureStatsVq       = 0x2;
inline constexpr std::uint32_t kFeatureDeflateOnOom = 0x4;

inline constexpr std::uint32_t kPfnShift = 12;

inline constexpr std::uint32_t kStatSwapIn  = 0;
inline constexpr std::uint32_t kStatSwapOut = 1;
inline constexpr std::uint32_t kStatMajflt   = 2;
inline constexpr std::uint32_t kStatMinflt   = 3;
inline constexpr std::uint32_t kStatMemfree  = 4;
inline constexpr std::uint32_t kStatMemtot   = 5;
inline constexpr std::uint32_t kStatAvail    = 6;
inline constexpr std::uint32_t kStatCaches   = 7;
inline constexpr std::uint32_t kStatNr       = 8;

inline constexpr RegInsn kVirtioBalloonInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioBalloonReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_balloon_1_0() noexcept {
    return Descriptor{
        .name = "virtio-balloon-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/balloon/virtio_balloon.h",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevBalloon,
        .init_sequence = kVirtioBalloonInit,
        .reset_sequence = kVirtioBalloonReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevBalloon;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdBalloon);
}

} // namespace pbsd::uda::virtio::balloon
