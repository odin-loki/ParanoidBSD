export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_setprio;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_setprio.c
// void thr_setprio_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_setprio.c wave=wave2 loc=66
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_setprio {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_setprio
