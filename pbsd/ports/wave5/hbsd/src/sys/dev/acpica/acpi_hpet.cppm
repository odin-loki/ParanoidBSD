export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_hpet;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_hpet.c
// void acpi_hpet_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_hpet.c wave=wave5 loc=1010
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_hpet {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_hpet
