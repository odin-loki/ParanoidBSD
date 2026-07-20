export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.pci_uart;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/pci_uart.c
// void pci_uart_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/pci_uart.c wave=wave2 loc=129
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_uart {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_uart
