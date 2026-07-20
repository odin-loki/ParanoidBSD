export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_spinlock;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_spinlock.c
// void thr_spinlock_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: converted
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_spinlock.c wave=wave2 loc=123
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_spinlock {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_spinlock
