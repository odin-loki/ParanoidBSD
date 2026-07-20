export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_pspinlock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_pspinlock.c
// void thr_pspinlock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_pspinlock.c wave=wave2 loc=153
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_pspinlock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_pspinlock
