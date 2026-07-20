export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_setschedparam;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_setschedparam.c
// void thr_setschedparam_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_setschedparam.c wave=wave2 loc=77
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_setschedparam {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_setschedparam
