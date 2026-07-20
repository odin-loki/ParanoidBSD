export module pbsd.port.wave2.hbsd.src.lib.libthr.thread.thr_info;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/thread/thr_info.c
// void thr_info_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/thread/thr_info.c wave=wave2 loc=151
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_info {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::thread::thr_info
