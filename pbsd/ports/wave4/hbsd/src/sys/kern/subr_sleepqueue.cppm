export module pbsd.port.wave4.hbsd.src.sys.kern.subr_sleepqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_sleepqueue.c
// void subr_sleepqueue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_sleepqueue.c wave=wave4 loc=1508
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sleepqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_sleepqueue
