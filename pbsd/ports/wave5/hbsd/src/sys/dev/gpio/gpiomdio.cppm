export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiomdio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiomdio.c
// void gpiomdio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiomdio.c wave=wave5 loc=238
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiomdio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiomdio
