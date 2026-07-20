export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_quirk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_quirk.c
// void acpi_quirk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_quirk.c wave=wave5 loc=182
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_quirk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_quirk
