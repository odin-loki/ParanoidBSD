export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.ofw_gpiobus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/ofw_gpiobus.c
// void ofw_gpiobus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/ofw_gpiobus.c wave=wave5 loc=510
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::ofw_gpiobus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::ofw_gpiobus
