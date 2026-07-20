export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_dock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_dock.c
// void acpi_dock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_dock.c wave=wave5 loc=546
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_dock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_dock
