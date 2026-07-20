export module pbsd.port.wave4.hbsd.src.sys.kern.subr_busdma_bufalloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_busdma_bufalloc.c
// void subr_busdma_bufalloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_busdma_bufalloc.c wave=wave4 loc=167
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_busdma_bufalloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_busdma_bufalloc
