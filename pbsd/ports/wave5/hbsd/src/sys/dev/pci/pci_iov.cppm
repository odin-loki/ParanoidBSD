export module pbsd.port.wave5.hbsd.src.sys.dev.pci.pci_iov;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/pci/pci_iov.c
// void pci_iov_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/pci/pci_iov.c wave=wave5 loc=1257
export namespace pbsd::port::wave5::hbsd::src::sys::dev::pci::pci_iov {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::pci::pci_iov
