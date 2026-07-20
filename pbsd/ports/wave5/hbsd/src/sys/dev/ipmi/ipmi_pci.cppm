export module pbsd.port.wave5.hbsd.src.sys.dev.ipmi.ipmi_pci;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/ipmi/ipmi_pci.c
// void ipmi_pci_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/ipmi/ipmi_pci.c wave=wave5 loc=299
export namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_pci {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::ipmi::ipmi_pci
