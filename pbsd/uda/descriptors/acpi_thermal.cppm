module;
#include <cstdint>

export module pbsd.uda.acpi_thermal;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_thermal.c
export namespace pbsd::uda::acpi_thermal {

inline constexpr std::uint8_t kNotifyTemperature = 0x80;
inline constexpr std::uint8_t kNotifyLevels      = 0x81;
inline constexpr std::uint8_t kNotifyDevices     = 0x82;
inline constexpr std::uint8_t kNotifyCritical    = 0xCC;

inline constexpr std::int32_t kTzZeroc = 2731;
inline constexpr std::uint8_t kTzNumLevels = 10;
inline constexpr std::uint8_t kTzPollRateSec = 10;
inline constexpr std::uint8_t kTzValidChecks = 3;

inline constexpr RegInsn kAcpiThermalInit[] = {
    {RegOp::Write32, 0x00, kTzZeroc, 0, 0},
    {RegOp::Write8, 0x04, kTzPollRateSec, 0, 0},
    {RegOp::Write8, 0x05, kTzValidChecks, 0, 0},
    {RegOp::Write8, 0x06, kTzNumLevels, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiThermalReset[] = {
    {RegOp::Write8, 0x04, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_thermal_zone() noexcept {
    return Descriptor{
        .name = "acpi-thermal-zone",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_thermal.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiThermalInit,
        .reset_sequence = kAcpiThermalReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return mem.read32(0x00) >= kTzZeroc;
}

} // namespace pbsd::uda::acpi_thermal
