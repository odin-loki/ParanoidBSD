export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_create;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_create.c
// void thr_create_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_create.c wave=wave2 loc=303
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_create {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_create
