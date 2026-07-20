export module pbsd.port.wave5.hbsd.src.sys.dev.uart.uart_cpu_x86;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/uart/uart_cpu_x86.c
// void uart_cpu_x86_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/uart/uart_cpu_x86.c wave=wave5 loc=121
export namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_cpu_x86 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::uart::uart_cpu_x86
