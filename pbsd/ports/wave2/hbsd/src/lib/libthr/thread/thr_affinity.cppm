export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_affinity;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_affinity.c
// void thr_affinity_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_affinity.c wave=wave2 loc=84
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_affinity {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_affinity
