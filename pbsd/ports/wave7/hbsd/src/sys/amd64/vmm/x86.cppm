export module pbsd.port.wave7.hbsd.src.sys.amd64.vmm.x86;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/amd64/vmm/x86.c
// void x86_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/vmm/x86.c wave=wave7 loc=769
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::x86 {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::vmm::x86
