export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.intel.vmx;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/intel/vmx.c
// void vmx_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/intel/vmx.c wave=wave7 loc=4307
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::intel::vmx {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::intel::vmx
