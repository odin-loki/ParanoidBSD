export module pbsd.port.wave4.hbsd.src.sys.compat.linuxkpi.common.src.linux_pci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/compat/linuxkpi/common/src/linux_pci.c
// void linux_pci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/compat/linuxkpi/common/src/linux_pci.c wave=wave4 loc=2245
export namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_pci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::compat::linuxkpi::common::src::linux_pci
