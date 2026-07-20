export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_kill;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_kill.c
// void thr_kill_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_kill.c wave=wave2 loc=74
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_kill {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_kill
