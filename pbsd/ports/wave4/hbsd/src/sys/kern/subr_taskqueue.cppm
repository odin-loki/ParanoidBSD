export module pbsd.port.wave4.hbsd.src.sys.kern.subr_taskqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_taskqueue.c
// void subr_taskqueue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_taskqueue.c wave=wave4 loc=918
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_taskqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_taskqueue
