module;
#include <cstdint>

export module pbsd.uda.smart;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/cam/scsi/scsi_da.c (SMART ATA pass-through), ata.h
export namespace pbsd::uda::smart {

inline constexpr std::uint8_t kAtaSmartEnable  = 0xD8;
inline constexpr std::uint8_t kAtaSmartDisable = 0xD9;
inline constexpr std::uint8_t kAtaSmartStatus  = 0xDA;
inline constexpr std::uint8_t kAtaSmartReadData = 0xD0;

inline constexpr RegInsn kSmartInit[] = {
    {RegOp::Write8, 0x00, kAtaSmartEnable, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write8, 0x01, kAtaSmartStatus, 0, 0},
    {RegOp::Write8, 0x02, kAtaSmartReadData, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSmartReset[] = {
    {RegOp::Write8, 0x00, kAtaSmartDisable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor smart_ata() noexcept {
    return Descriptor{
        .name = "smart-ata",
        .provenance = "hbsd/src/sys/cam/ata/ata_all.h",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kSmartInit,
        .reset_sequence = kSmartReset,
    };
}

} // namespace pbsd::uda::smart
