export module pbsd.port.wave5.hbsd.src.sys.dev.gpio.gpiospi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/gpio/gpiospi.c
// void gpiospi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/gpio/gpiospi.c wave=wave5 loc=401
export namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiospi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::gpio::gpiospi
