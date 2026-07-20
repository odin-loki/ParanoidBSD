export module pbsd.port.wave2.hbsd.src.lib.libc.stdtime.timelocal;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/stdtime/timelocal.c
// void timelocal_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/stdtime/timelocal.c wave=wave2 loc=153
export namespace pbsd::port::wave2::hbsd::src::lib::libc::stdtime::timelocal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::stdtime::timelocal
