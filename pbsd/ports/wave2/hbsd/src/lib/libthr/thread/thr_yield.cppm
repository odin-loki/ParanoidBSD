export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_yield;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_yield.c
// void thr_yield_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_yield.c wave=wave2 loc=45
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_yield {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_yield
