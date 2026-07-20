export module pbsd.port.wave7.hbsd.src.sys.arm64.apple.exynos_uart;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/arm64/apple/exynos_uart.c
// void exynos_uart_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/apple/exynos_uart.c wave=wave7 loc=568
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::apple::exynos_uart {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::apple::exynos_uart
