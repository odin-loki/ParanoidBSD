export module pbsd.port.wave5.hbsd.src.sys.dev.pci.pci_iov_schema;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/pci/pci_iov_schema.c
// void pci_iov_schema_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/pci/pci_iov_schema.c wave=wave5 loc=926
export namespace pbsd::port::wave5::hbsd::src::sys::dev::pci::pci_iov_schema {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::pci::pci_iov_schema
