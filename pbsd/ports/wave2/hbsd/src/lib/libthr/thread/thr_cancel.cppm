export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_cancel;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_cancel.c
// void thr_cancel_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_cancel.c wave=wave2 loc=183
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_cancel {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_cancel
