export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.vmm_ioport;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/vmm_ioport.c
// void vmm_ioport_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/vmm_ioport.c wave=wave7 loc=216
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_ioport {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_ioport
