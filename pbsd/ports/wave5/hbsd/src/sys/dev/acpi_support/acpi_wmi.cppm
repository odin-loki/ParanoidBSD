export module pbsd.port.wave5.hbsd.src.sys.dev.acpi_support.acpi_wmi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpi_support/acpi_wmi.c
// void acpi_wmi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpi_support/acpi_wmi.c wave=wave5 loc=1055
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpi_support::acpi_wmi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpi_support::acpi_wmi
