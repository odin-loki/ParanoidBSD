export module pbsd.port.wave4.hbsd.src.sys.kern.subr_fattime;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/subr_fattime.c
// void subr_fattime_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/subr_fattime.c wave=wave4 loc=309
export namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_fattime {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::subr_fattime
