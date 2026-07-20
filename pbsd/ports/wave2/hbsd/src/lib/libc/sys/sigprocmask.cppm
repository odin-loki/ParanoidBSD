export module pbsd.port.wave2.hbsd.src.lib.libc.sys.sigprocmask;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/sigprocmask.c
// void sigprocmask_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/sigprocmask.c wave=wave2 loc=43
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigprocmask {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigprocmask
