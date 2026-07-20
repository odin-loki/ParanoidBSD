export module pbsd.port.wave5.hbsd.src.sys.dev.acpica.acpi_throttle;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/acpica/acpi_throttle.c
// void acpi_throttle_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/acpica/acpi_throttle.c wave=wave5 loc=440
export namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_throttle {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::acpica::acpi_throttle
