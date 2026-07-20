export module pbsd.port.wave5.hbsd.src.sys.dev.fdc.fdc_acpi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/fdc/fdc_acpi.c
// void fdc_acpi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/fdc/fdc_acpi.c wave=wave5 loc=274
export namespace pbsd::port::wave5::hbsd::src::sys::dev::fdc::fdc_acpi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::fdc::fdc_acpi
