export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_suspend_np;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_suspend_np.c
// void thr_suspend_np_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_suspend_np.c wave=wave2 loc=184
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_suspend_np {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_suspend_np
