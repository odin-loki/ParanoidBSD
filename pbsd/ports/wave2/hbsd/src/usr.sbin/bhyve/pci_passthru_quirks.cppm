export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyve.pci_passthru_quirks;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyve/pci_passthru_quirks.c
// void pci_passthru_quirks_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyve/pci_passthru_quirks.c wave=wave2 loc=48
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_passthru_quirks {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyve::pci_passthru_quirks
