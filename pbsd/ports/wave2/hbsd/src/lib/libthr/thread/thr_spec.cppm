export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_spec;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_spec.c
// void thr_spec_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_spec.c wave=wave2 loc=244
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_spec {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_spec
