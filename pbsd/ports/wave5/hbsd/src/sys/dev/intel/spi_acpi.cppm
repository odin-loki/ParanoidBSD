export module pbsd.port.wave5.hbsd.src.sys.dev.intel.spi_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/intel/spi_acpi.c
// void spi_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/intel/spi_acpi.c wave=wave5 loc=120
export namespace pbsd::port::wave5::hbsd::src::sys::dev::intel::spi_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::intel::spi_acpi
