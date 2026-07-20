export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiopower;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiopower.c
// void gpiopower_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiopower.c wave=wave5 loc=153
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiopower {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiopower
