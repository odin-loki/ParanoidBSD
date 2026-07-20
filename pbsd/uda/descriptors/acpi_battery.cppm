module;
#include <cstdint>

export module pbsd.uda.acpi_battery;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_battery.c
export namespace pbsd::uda::acpi_battery {

inline constexpr std::uint8_t kNotifyStatus = 0x80;
inline constexpr std::uint8_t kNotifyInfo   = 0x81;

inline constexpr std::uint8_t kStateDischarging = 0x01;
inline constexpr std::uint8_t kStateCharging    = 0x02;
inline constexpr std::uint8_t kStateCritical    = 0x04;

inline constexpr RegInsn kAcpiBatteryInit[] = {
    {RegOp::Write8, 0x00, kStateDischarging, 0, 0},
    {RegOp::Write8, 0x01, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiBatteryReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_battery_device() noexcept {
    return Descriptor{
        .name = "acpi-battery",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_battery.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiBatteryInit,
        .reset_sequence = kAcpiBatteryReset,
    };
}

} // namespace pbsd::uda::acpi_battery
