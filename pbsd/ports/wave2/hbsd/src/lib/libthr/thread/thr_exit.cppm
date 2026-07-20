export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_exit;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_exit.c
// void thr_exit_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_exit.c wave=wave2 loc=327
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_exit {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_exit
