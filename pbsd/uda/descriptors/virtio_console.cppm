module;
#include <cstdint>

export module pbsd.uda.virtio_console;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.virtio.common;

/// PROVENANCE: hbsd/src/sys/dev/virtio/console/virtio_console.h
export namespace pbsd::uda::virtio::console {

inline constexpr std::uint32_t kFeatureSize      = 0x01;
inline constexpr std::uint32_t kFeatureMultiport = 0x02;
inline constexpr std::uint32_t kFeatureEmergWrite = 0x04;

inline constexpr std::uint32_t kEventDeviceReady = 0;
inline constexpr std::uint32_t kEventPortAdd     = 1;
inline constexpr std::uint32_t kEventPortOpen    = 6;

inline constexpr RegInsn kVirtioConsoleInit[] = {
    {RegOp::Write32, kMmioStatus, kStatusAck, 0, 0},
    {RegOp::Write32, kMmioStatus, kStatusAck | kStatusDriver, 0, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk, 0, 0},
    {RegOp::CheckEq, kMmioStatus, kFeaturesOk, kFeaturesOk, 0},
    {RegOp::Write32, kMmioStatus,
     kStatusAck | kStatusDriver | kFeaturesOk | kStatusDriverOk, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVirtioConsoleReset[] = {
    {RegOp::Write32, kMmioStatus, kStatusReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor virtio_console_1_0() noexcept {
    return Descriptor{
        .name = "virtio-console-1.0",
        .provenance = "hbsd/src/sys/dev/virtio/console/virtio_console.h",
        .device_class = DeviceClass::Input,
        .vendor_id = kPciVendorRedHat,
        .device_id = kPciDevConsole,
        .init_sequence = kVirtioConsoleInit,
        .reset_sequence = kVirtioConsoleReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRedHat && device == kPciDevConsole;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return virtio::probe_mmio_device(mem, kIdConsole);
}

} // namespace pbsd::uda::virtio::console
