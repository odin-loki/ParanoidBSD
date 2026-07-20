export module pbsd.port.wave5.hbsd.src.sys.dev.acpi_support.acpi_ibm;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpi_support/acpi_ibm.c
// void acpi_ibm_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpi_support/acpi_ibm.c wave=wave5 loc=1553
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpi_support::acpi_ibm {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpi_support::acpi_ibm
