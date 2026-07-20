export module pbsd.port.wave2.hbsd.src.lib.libc.sys.sigsuspend;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/sigsuspend.c
// void sigsuspend_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/sigsuspend.c wave=wave2 loc=43
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigsuspend {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::sigsuspend
