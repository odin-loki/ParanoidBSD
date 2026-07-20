export module pbsd.port.wave5.hbsd.src.sys.dev.virtio.pci.virtio_pci_legacy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/virtio/pci/virtio_pci_legacy.c
// void virtio_pci_legacy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/virtio/pci/virtio_pci_legacy.c wave=wave5 loc=764
export namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::pci::virtio_pci_legacy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::virtio::pci::virtio_pci_legacy
