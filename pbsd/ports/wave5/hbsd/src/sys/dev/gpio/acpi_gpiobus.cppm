export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.acpi_gpiobus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/acpi_gpiobus.c
// void acpi_gpiobus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/acpi_gpiobus.c wave=wave5 loc=449
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::acpi_gpiobus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::acpi_gpiobus
