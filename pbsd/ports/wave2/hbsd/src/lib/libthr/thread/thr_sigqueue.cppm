export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_sigqueue;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_sigqueue.c
// void thr_sigqueue_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_sigqueue.c wave=wave2 loc=79
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_sigqueue {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_sigqueue
