export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_malloc.c
// void thr_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_malloc.c wave=wave2 loc=160
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_malloc
