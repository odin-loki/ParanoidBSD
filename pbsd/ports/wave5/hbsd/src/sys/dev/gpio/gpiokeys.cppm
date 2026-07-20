export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiokeys;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiokeys.c
// void gpiokeys_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiokeys.c wave=wave5 loc=1056
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiokeys {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiokeys
