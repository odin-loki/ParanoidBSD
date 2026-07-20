export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.vmm_mem;

module;
// Header bridge — replace #include of hbsd/src/sys/amd64/vmm/vmm_mem.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/vmm_mem.h wave=wave7 loc=39
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_mem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_mem
