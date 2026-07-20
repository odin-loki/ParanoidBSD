module;
#include <cstdint>

export module pbsd.uda.acpi_ec;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_ec.c
export namespace pbsd::uda::acpi_ec {

inline constexpr std::uint8_t kRegStatus  = 0x00;
inline constexpr std::uint8_t kRegData    = 0x01;
inline constexpr std::uint8_t kRegCommand  = 0x02;
inline constexpr std::uint8_t kStatusObf = 0x01;
inline constexpr std::uint8_t kStatusIbf = 0x02;

inline constexpr RegInsn kAcpiEcInit[] = {
    {RegOp::Read8, kRegStatus, 0, 0, 0},
    {RegOp::Write8, kRegCommand, 0x84, 0, 0},
    {RegOp::CheckEq, kRegStatus, 0, kStatusObf, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiEcReset[] = {
    {RegOp::Write8, kRegData, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_embedded_controller() noexcept {
    return Descriptor{
        .name = "acpi-ec",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_ec.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiEcInit,
        .reset_sequence = kAcpiEcReset,
    };
}

} // namespace pbsd::uda::acpi_ec
