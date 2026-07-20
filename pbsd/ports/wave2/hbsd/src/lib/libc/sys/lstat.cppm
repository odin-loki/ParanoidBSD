export module pbsd.port.wave2.hbsd.src.lib.libc.sys.lstat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/sys/lstat.c
// void lstat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/sys/lstat.c wave=wave2 loc=40
export namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::lstat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::sys::lstat
