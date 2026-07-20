export module pbsd.port.wave4.hbsd.src.sys.kern.subr_physmem;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_physmem.c
// void subr_physmem_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_physmem.c wave=wave4 loc=627
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_physmem {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_physmem
