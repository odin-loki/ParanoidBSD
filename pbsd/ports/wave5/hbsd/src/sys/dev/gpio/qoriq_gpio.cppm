export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.qoriq_gpio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/qoriq_gpio.c
// void qoriq_gpio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/qoriq_gpio.c wave=wave5 loc=438
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::qoriq_gpio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::qoriq_gpio
