export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.amd.vmcb;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/amd/vmcb.c
// void vmcb_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/amd/vmcb.c wave=wave7 loc=561
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::amd::vmcb {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::amd::vmcb
