export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiobus;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiobus.c
// void gpiobus_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiobus.c wave=wave5 loc=1193
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiobus {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiobus
