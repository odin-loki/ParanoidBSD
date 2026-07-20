export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.pci_virtio_console;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/pci_virtio_console.c
// void pci_virtio_console_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/pci_virtio_console.c wave=wave2 loc=764
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_console {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_console
