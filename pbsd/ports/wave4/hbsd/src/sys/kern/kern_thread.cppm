export module pbsd.port.wave4.hbsd.src.sys.kern.kern_thread;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/kern_thread.c
// void kern_thread_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/kern_thread.c wave=wave4 loc=1825
export namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_thread {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::kern_thread
