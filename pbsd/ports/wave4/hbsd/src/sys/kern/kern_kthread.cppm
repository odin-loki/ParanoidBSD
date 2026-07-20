export module pbsd.port.wave4.hbsd.src.sys.kern.kern_kthread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_kthread.c
// void kern_kthread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_kthread.c wave=wave4 loc=526
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_kthread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_kthread
