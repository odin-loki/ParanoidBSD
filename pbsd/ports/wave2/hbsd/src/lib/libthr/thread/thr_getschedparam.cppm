export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_getschedparam;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_getschedparam.c
// void thr_getschedparam_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_getschedparam.c wave=wave2 loc=68
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_getschedparam {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_getschedparam
