module;
#include <cstdint>

export module pbsd.uda.acpi_button;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_button.c
export namespace pbsd::uda::acpi_button {

inline constexpr std::uint8_t kNotifySleep  = 0x80;
inline constexpr std::uint8_t kNotifyWakeup = 0x02;

inline constexpr std::uint8_t kTypePower = 0;
inline constexpr std::uint8_t kTypeSleep  = 1;

inline constexpr RegInsn kAcpiButtonInit[] = {
    {RegOp::Write8, 0x00, kTypePower, 0, 0},
    {RegOp::Write8, 0x01, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiButtonReset[] = {
    {RegOp::Write8, 0x00, 0xFF, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_power_button() noexcept {
    return Descriptor{
        .name = "acpi-power-button",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_button.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiButtonInit,
        .reset_sequence = kAcpiButtonReset,
    };
}

} // namespace pbsd::uda::acpi_button
