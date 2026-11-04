module;
#include <cstdint>

export module pbsd.uda.umass;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/storage/umass.c
export namespace pbsd::uda::umass {

inline constexpr std::uint8_t kUmassMaxLun = 0x0F;
inline constexpr std::uint8_t kReqGetMaxLun = 0xFE;
inline constexpr std::uint8_t kReqBulkReset = 0xFF;

inline constexpr RegInsn kUmassInit[] = {
    {RegOp::Write8, 0x00, kReqGetMaxLun, 0, 0},
    {RegOp::Write8, 0x01, kUmassMaxLun, 0, 0},
    {RegOp::Write8, 0x02, kReqBulkReset, 0, 0},
    {RegOp::Write8, 0x03, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUmassReset[] = {
    {RegOp::Write8, 0x00, kReqBulkReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor umass_scsi() noexcept {
    return Descriptor{
        .name = "umass-scsi",
        .provenance = "hbsd/src/sys/dev/usb/storage/umass.c",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kUmassInit,
        .reset_sequence = kUmassReset,
    };
}

} // namespace pbsd::uda::umass
