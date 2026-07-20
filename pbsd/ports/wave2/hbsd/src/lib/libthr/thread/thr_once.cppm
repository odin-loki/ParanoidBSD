export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_once;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_once.c
// void thr_once_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_once.c wave=wave2 loc=103
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_once {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_once
