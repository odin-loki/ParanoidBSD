export module pbsd.port.wave5.hbsd.src.sys.dev.uart.uart_dev_z8530;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/uart/uart_dev_z8530.c
// void uart_dev_z8530_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/uart/uart_dev_z8530.c wave=wave5 loc=650
export namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_dev_z8530 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_dev_z8530
