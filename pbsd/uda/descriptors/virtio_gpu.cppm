module;
#include <cstdint>

export module pbsd.uda.virtio_gpu;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/gpu/virtio_gpu.h
export namespace pbsd::uda::virtio::gpu {

inline constexpr std::uint32_t kFeatureVirgl         = 0;
inline constexpr std::uint32_t kFeatureEdid          = 1;
inline constexpr std::uint32_t kFeatureResourceUuid  = 2;
inline constexpr std::uint32_t kFeatureResourceBlob  = 3;
inline constexpr std::uint32_t kFeatureContextInit   = 4;

inline constexpr std::uint32_t kCmdGetDisplayInfo    = 0x0100;
inline constexpr std::uint32_t kCmdResourceCreate2d  = 0x0101;
inline constexpr std::uint32_t kCmdSetScanout        = 0x0103;
inline constexpr std::uint32_t kCmdResourceFlush     = 0x0104;
inline constexpr std::uint32_t kCmdTransferToHost2d  = 0x0105;
inline constexpr std::uint32_t kCmdGetEdid           = 0x010A;

inline constexpr std::uint32_t kRespOkNodata         = 0x1100;
inline constexpr std::uint32_t kRespOkDisplayInfo    = 0x1101;
inline constexpr std::uint32_t kRespOkEdid           = 0x1105;

inline constexpr std::uint32_t kMaxScanouts          = 16;
inline constexpr std::uint32_t kFormatB8g8r8a8Unorm = 1;

inline constexpr RegInsn kVirtioGpuInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioGpuReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_gpu_1_0() noexcept {
    return Descriptor{
        .name = "virtio-gpu-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/gpu/virtio_gpu.h",
        .device_class = DeviceClass::Display,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevGpu,
        .init_sequence = kVirtioGpuInit,
        .reset_sequence = kVirtioGpuReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevGpu;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdGpu);
}

} // namespace pbsd::uda::virtio::gpu
