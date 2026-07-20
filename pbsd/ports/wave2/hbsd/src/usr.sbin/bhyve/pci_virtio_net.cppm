export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.pci_virtio_net;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/pci_virtio_net.c
// void pci_virtio_net_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/pci_virtio_net.c wave=wave2 loc=815
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_net {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_virtio_net
