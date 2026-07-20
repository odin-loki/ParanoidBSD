export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.pci_virtio_input;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/pci_virtio_input.c
// void pci_virtio_input_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/pci_virtio_input.c wave=wave2 loc=774
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_input {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_input
