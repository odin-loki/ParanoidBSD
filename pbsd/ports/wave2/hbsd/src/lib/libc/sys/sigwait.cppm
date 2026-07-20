export module pbsd.port.wave2.hbsd.src.lib.libc.sys.sigwait;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/sigwait.c
// void sigwait_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/sigwait.c wave=wave2 loc=36
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigwait {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigwait
