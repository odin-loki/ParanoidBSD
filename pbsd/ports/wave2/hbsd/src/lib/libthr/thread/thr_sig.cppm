export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_sig;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_sig.c
// void thr_sig_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_sig.c wave=wave2 loc=825
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_sig {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_sig
