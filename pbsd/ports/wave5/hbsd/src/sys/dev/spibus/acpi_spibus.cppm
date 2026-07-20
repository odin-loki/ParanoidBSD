export module pbsd.port.wave5.hbsd.src.sys.dev.spibus.acpi_spibus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/spibus/acpi_spibus.c
// void acpi_spibus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/spibus/acpi_spibus.c wave=wave5 loc=581
export namespace pbsd::port::wave5::hbsd::src::sys::dev::spibus::acpi_spibus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::spibus::acpi_spibus
