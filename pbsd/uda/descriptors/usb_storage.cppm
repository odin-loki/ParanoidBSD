module;
#include <cstdint>

export module pbsd.uda.usb_storage;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/storage/umass.c, usb_storage protocol.
export namespace pbsd::uda::usb_storage {

inline constexpr std::uint8_t kCbwSignature0 = 0x55;
inline constexpr std::uint8_t kCbwSignature1 = 0x53;
inline constexpr std::uint8_t kCbwSignature2 = 0x42;
inline constexpr std::uint8_t kCbwSignature3 = 0x43;

inline constexpr std::uint8_t kCswSignature0 = 0x55;
inline constexpr std::uint8_t kCswSignature1 = 0x53;
inline constexpr std::uint8_t kCswSignature2 = 0x42;
inline constexpr std::uint8_t kCswSignature3 = 0x53;

inline constexpr std::uint8_t kCswStatusPassed = 0x00;

inline constexpr RegInsn kUsbStorageInit[] = {
    {RegOp::Write8, 0x00, kCbwSignature0, 0, 0},
    {RegOp::Write8, 0x01, kCbwSignature1, 0, 0},
    {RegOp::Write8, 0x02, kCbwSignature2, 0, 0},
    {RegOp::Write8, 0x03, kCbwSignature3, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUsbStorageReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor usb_storage_bulk() noexcept {
    return Descriptor{
        .name = "usb-storage-bulk",
        .provenance = "hbsd/src/sys/dev/usb/storage/umass.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kUsbStorageInit,
        .reset_sequence = kUsbStorageReset,
    };
}

} // namespace pbsd::uda::usb_storage
