export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_thermal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_thermal.c
// void acpi_thermal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_thermal.c wave=wave5 loc=1235
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_thermal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_thermal
