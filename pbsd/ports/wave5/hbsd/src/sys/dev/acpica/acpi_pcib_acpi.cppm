export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_pcib_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_pcib_acpi.c
// void acpi_pcib_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_pcib_acpi.c wave=wave5 loc=717
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcib_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcib_acpi
