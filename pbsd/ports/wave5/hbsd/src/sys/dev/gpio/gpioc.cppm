export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpioc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpioc.c
// void gpioc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpioc.c wave=wave5 loc=1086
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpioc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpioc
