export module pbsd.port.wave2.hbsd.src.lib.libthr.sys.thr_error;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libthr/sys/thr_error.c
// void thr_error_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libthr/sys/thr_error.c wave=wave2 loc=56
export namespace pbsd::port::wave2::hbsd::src::lib::libthr::sys::thr_error {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libthr::sys::thr_error
