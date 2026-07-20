export module pbsd.port.wave2.hbsd.src.lib.libgpio.gpio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libgpio/gpio.c
// void gpio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libgpio/gpio.c wave=wave2 loc=296
export namespace pbsd::port::wave2::hbsd::src::lib::libgpio::gpio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libgpio::gpio
