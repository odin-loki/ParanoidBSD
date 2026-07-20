export module pbsd.port.wave2.hbsd.src.usr_bin.ktrace.subr;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/ktrace/subr.c
// void subr_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/ktrace/subr.c wave=wave2 loc=131
export namespace pbsd::port::wave2::hbsd::src::usr_bin::ktrace::subr {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::ktrace::subr
