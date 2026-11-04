module;
#include <cstdint>

export module pbsd.uda.acpi_lid;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_lid.c
export namespace pbsd::uda::acpi_lid {

inline constexpr std::uint8_t kNotifyStatus = 0x80;
inline constexpr std::uint8_t kStateOpen    = 0x00;
inline constexpr std::uint8_t kStateClosed   = 0x01;

inline constexpr RegInsn kAcpiLidInit[] = {
    {RegOp::Write8, 0x00, kStateOpen, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiLidReset[] = {
    {RegOp::Write8, 0x00, kStateClosed, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_lid_switch() noexcept {
    return Descriptor{
        .name = "acpi-lid",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_lid.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiLidInit,
        .reset_sequence = kAcpiLidReset,
    };
}

} // namespace pbsd::uda::acpi_lid
