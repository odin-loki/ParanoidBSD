export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_perf;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_perf.c
// void acpi_perf_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_perf.c wave=wave5 loc=593
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_perf {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_perf
