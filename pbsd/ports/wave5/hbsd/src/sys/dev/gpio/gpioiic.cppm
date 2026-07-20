export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpioiic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpioiic.c
// void gpioiic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpioiic.c wave=wave5 loc=373
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpioiic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpioiic
