export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_apei;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_apei.c
// void acpi_apei_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_apei.c wave=wave5 loc=828
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_apei {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_apei
