export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiopps;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiopps.c
// void gpiopps_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiopps.c wave=wave5 loc=285
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiopps {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiopps
