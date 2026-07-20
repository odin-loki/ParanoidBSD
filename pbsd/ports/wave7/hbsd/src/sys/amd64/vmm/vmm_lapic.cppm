export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.vmm_lapic;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/vmm_lapic.c
// void vmm_lapic_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/vmm_lapic.c wave=wave7 loc=239
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_lapic {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::vmm_lapic
