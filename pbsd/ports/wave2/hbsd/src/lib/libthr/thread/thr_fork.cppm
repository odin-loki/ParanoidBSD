export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_fork;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_fork.c
// void thr_fork_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_fork.c wave=wave2 loc=331
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_fork {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_fork
