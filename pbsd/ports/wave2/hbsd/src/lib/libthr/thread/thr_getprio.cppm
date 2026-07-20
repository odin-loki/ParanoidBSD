export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_getprio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_getprio.c
// void thr_getprio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_getprio.c wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_getprio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_getprio
