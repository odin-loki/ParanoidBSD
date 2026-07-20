module;
#include <cstdint>

export module pbsd.uda.virtio_random;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/random/virtio_random.c, virtio_ids.h
export namespace pbsd::uda::virtio::random {

inline constexpr std::uint64_t kFeatures = 0;

inline constexpr RegInsn kVirtioRandomInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioRandomReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_random_1_0() noexcept {
    return Descriptor{
        .name = "virtio-random-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/random/virtio_random.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevEntropy,
        .init_sequence = kVirtioRandomInit,
        .reset_sequence = kVirtioRandomReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevEntropy;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdEntropy);
}

} // namespace pbsd::uda::virtio::random
