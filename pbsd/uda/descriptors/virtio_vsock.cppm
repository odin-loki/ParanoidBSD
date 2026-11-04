module;
#include <cstdint>

export module pbsd.uda.virtio_vsock;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/vsock/virtio_vsock.c, virtio_ids.h
export namespace pbsd::uda::virtio::vsock {

inline constexpr std::uint32_t kIdVsock = 19;
inline constexpr std::uint16_t kPciDevVsock = 0x1014;

inline constexpr RegInsn kVirtioVsockInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioVsockReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_vsock_1_0() noexcept {
    return Descriptor{
        .name = "virtio-vsock-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/vsock/virtio_vsock.c",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevVsock,
        .init_sequence = kVirtioVsockInit,
        .reset_sequence = kVirtioVsockReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdVsock);
}

} // namespace pbsd::uda::virtio::vsock
