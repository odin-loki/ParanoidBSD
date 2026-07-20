export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_isab;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_isab.c
// void acpi_isab_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_isab.c wave=wave5 loc=127
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_isab {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_isab
