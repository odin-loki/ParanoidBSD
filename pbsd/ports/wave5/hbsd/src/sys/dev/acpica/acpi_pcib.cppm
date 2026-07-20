export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_pcib;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_pcib.c
// void acpi_pcib_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_pcib.c wave=wave5 loc=338
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcib {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_pcib
