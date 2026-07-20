module;
#include <cstdint>

export module pbsd.uda.virtio_crypto;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/crypto/virtio_crypto.c, virtio_ids.h
export namespace pbsd::uda::virtio::crypto {

inline constexpr std::uint32_t kIdCrypto = 20;
inline constexpr std::uint16_t kPciDevCrypto = 0x1015;

inline constexpr RegInsn kVirtioCryptoInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioCryptoReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_crypto_1_0() noexcept {
    return Descriptor{
        .name = "virtio-crypto-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/crypto/virtio_crypto.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevCrypto,
        .init_sequence = kVirtioCryptoInit,
        .reset_sequence = kVirtioCryptoReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdCrypto);
}

} // namespace pbsd::uda::virtio::crypto
