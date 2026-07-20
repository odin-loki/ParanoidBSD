export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.chvgpio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/chvgpio.c
// void chvgpio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/chvgpio.c wave=wave5 loc=523
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::chvgpio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::chvgpio
