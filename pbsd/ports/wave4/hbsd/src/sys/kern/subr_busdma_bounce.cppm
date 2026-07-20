export module pbsd.port.wave4.hbsd.src.sys.kern.subr_busdma_bounce;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_busdma_bounce.c
// void subr_busdma_bounce_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_busdma_bounce.c wave=wave4 loc=549
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_busdma_bounce {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_busdma_bounce
