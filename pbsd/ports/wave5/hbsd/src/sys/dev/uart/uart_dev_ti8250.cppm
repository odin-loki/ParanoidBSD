export module pbsd.port.wave5.hbsd.src.sys.dev.uart.uart_dev_ti8250;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/uart/uart_dev_ti8250.c
// void uart_dev_ti8250_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/uart/uart_dev_ti8250.c wave=wave5 loc=127
export namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_dev_ti8250 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_dev_ti8250
